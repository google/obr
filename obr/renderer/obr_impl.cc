/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */

#include "obr/renderer/obr_impl.h"

#include <cstddef>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/synchronization/mutex.h"
#include "obr/ambisonic_binaural_decoder/ambisonic_binaural_decoder.h"
#include "obr/ambisonic_binaural_decoder/sh_hrir_creator.h"
#include "obr/ambisonic_encoder/ambisonic_encoder.h"
#include "obr/audio_buffer/audio_buffer.h"
#include "obr/common/constants.h"
#include "obr/peak_limiter/peak_limiter.h"
#include "obr/renderer/audio_element_config.h"
#include "obr/renderer/audio_element_type.h"

namespace obr {

namespace {

#define RETURN_IF_NOT_OK(...)             \
  do {                                    \
    absl::Status _status = (__VA_ARGS__); \
    if (!_status.ok()) return _status;    \
  } while (0)

}  // namespace

ObrImpl::ObrImpl(int buffer_size_per_channel, int sampling_rate)
    : buffer_size_per_channel_(buffer_size_per_channel),
      sampling_rate_(sampling_rate),
      head_tracking_enabled_(false),
      world_rotation_(WorldRotation()),
      fft_manager_(buffer_size_per_channel_) {
  CHECK_GT(buffer_size_per_channel_, 0);
  CHECK_GT(sampling_rate_, 0);
}

ObrImpl::~ObrImpl() = default;

absl::Status ObrImpl::ResetDsp() {
  LOG(INFO) << "Resetting DSP.";

  // Enable the lock.
  absl::MutexLock lock(&mutex_);

  // Release resources.
  ambisonic_binaural_decoder_.reset();
  sh_hrirs_L_.reset();
  sh_hrirs_R_.reset();
  ambisonic_encoder_.reset();
  peak_limiter_.reset();
  ambisonic_rotator_.reset();

  // Clear buffers.
  ambisonic_mix_bed_.Clear();

  return absl::OkStatus();
}

absl::Status ObrImpl::InitializeDsp() {
  // Check that the audio elements list is not empty.
  if (audio_elements_.empty()) {
    return absl::FailedPreconditionError(
        "No audio elements configured. Can't initialize DSP.");
  }

  // Analyze the list of requested audio elements and initialize the necessary
  // DSP.
  // For now, until rendering of multiple AEs is implemented, initialize DSP
  // using Ambisonic order of the first (and only) element.
  const size_t order = audio_elements_[0].GetBinauralFiltersAmbisonicOrder();
  const size_t number_of_input_channels = GetNumberOfInputChannels();

  CHECK_NE(order, 0);
  CHECK_GE(order, kMinSupportedAmbisonicOrder);
  CHECK_LE(order, kMaxSupportedAmbisonicOrder);

  if (GetNumberOfInputChannels() == 0) {
    return absl::FailedPreconditionError(
        "No input channels configured. Can't initialize DSP.");
  }

  // Reset the DSP.
  RETURN_IF_NOT_OK(ResetDsp());

  // Enable the lock.
  absl::MutexLock lock(&mutex_);

  // Setup Ambisonic mix bed.
  ambisonic_mix_bed_ =
      AudioBuffer((order + 1) * (order + 1), buffer_size_per_channel_);

  LOG(INFO) << "Initializing DSP:";
  LOG(INFO) << "  - Number of input channels: " << number_of_input_channels;
  LOG(INFO) << "  - Binaural filters Ambisonic order: " << order;
  LOG(INFO) << "  - Number of Ambisonic mix bed channels: "
            << ambisonic_mix_bed_.num_channels();

  // Initialize Ambisonic encoder.
  auto indices = GetAmbisonicEncoderSourceChannelIndices();
  if (!indices.empty()) {
    ambisonic_encoder_input_buffer_ =
        AudioBuffer(indices.size(), buffer_size_per_channel_);
    ambisonic_encoder_ =
        std::make_unique<AmbisonicEncoder>(indices.size(), order);
    RETURN_IF_NOT_OK(UpdateAmbisonicEncoder());
  }

  // Initialize HOA rotator.
  ambisonic_rotator_ = std::make_unique<AmbisonicRotator>(order);

  // Initialize Ambisonic binaural decoder.
  // Load filters matching the selected operational Ambisonic order.
  std::string order_string = std::to_string(order);
  sh_hrirs_L_ = CreateShHrirsFromAssets(order_string + "OA_L", sampling_rate_,
                                        &resampler_);
  sh_hrirs_R_ = CreateShHrirsFromAssets(order_string + "OA_R", sampling_rate_,
                                        &resampler_);

  CHECK_EQ(sh_hrirs_L_->num_channels(), sh_hrirs_R_->num_channels());
  CHECK_EQ(sh_hrirs_L_->num_frames(), sh_hrirs_R_->num_frames());

  ambisonic_binaural_decoder_ = std::make_unique<AmbisonicBinauralDecoder>(
      *sh_hrirs_L_, *sh_hrirs_R_, buffer_size_per_channel_, &fft_manager_);

  // Initialize peak limiter.
  peak_limiter_ = std::make_unique<PeakLimiter>(sampling_rate_, 50, -0.5);

  return absl::OkStatus();
}

void ObrImpl::Process(const AudioBuffer& input_buffer,
                      AudioBuffer* output_buffer) {
  CHECK_EQ(input_buffer.num_channels(), GetNumberOfInputChannels());
  CHECK_EQ(input_buffer.num_frames(), buffer_size_per_channel_);
  CHECK_NE(output_buffer, nullptr);
  CHECK_EQ(output_buffer->num_channels(), GetNumberOfOutputChannels());
  CHECK_EQ(output_buffer->num_frames(), buffer_size_per_channel_);

  // Enable the lock.
  absl::MutexLock lock(&mutex_);

  // Pass audio through Ambisonic Encoder and render to Ambisonic
  // mix bed.
  const auto indices = GetAmbisonicEncoderSourceChannelIndices();

  if (!indices.empty()) {
    // Create a copy of the input buffer with selected channels only.
    for (size_t i = 0; i < indices.size(); ++i) {
      ambisonic_encoder_input_buffer_[i] = input_buffer[indices[i]];
    }

    ambisonic_encoder_->ProcessPlanarAudioData(ambisonic_encoder_input_buffer_,
                                               &ambisonic_mix_bed_);
  } else {
    ambisonic_mix_bed_.Clear();
  }

  // Copy Ambisonic input channels to Ambisonic mix bed.
  for (const auto& audio_element : audio_elements_) {
    if (IsAmbisonicsType(audio_element.GetType())) {
      for (size_t channel = 0;
           channel < audio_element.GetNumberOfInputChannels(); ++channel) {
        ambisonic_mix_bed_[channel] +=
            input_buffer[audio_element.GetFirstChannelIndex() + channel];
      }
    }
  }

  if (head_tracking_enabled_) {
    // Pass Ambisonic mix bed through Ambisonic Rotator.
    ambisonic_rotator_->Process(world_rotation_, ambisonic_mix_bed_,
                          &ambisonic_mix_bed_);
  }

  // Pass Ambisonic mix bed through Ambisonic Binaural Decoder.
  ambisonic_binaural_decoder_->ProcessAudioBuffer(ambisonic_mix_bed_,
                                                  output_buffer);

  // Peak limit the output.
  peak_limiter_->Process(*output_buffer, output_buffer);
}

int ObrImpl::GetBufferSizePerChannel() const {
  return buffer_size_per_channel_;
}

int ObrImpl::GetSamplingRate() const { return sampling_rate_; }

size_t ObrImpl::GetNumberOfOutputChannels() { return kNumBinauralChannels; }

std::vector<size_t> ObrImpl::GetAmbisonicEncoderSourceChannelIndices() {
  std::vector<size_t> source_channel_indices;
  for (const auto& audio_element : audio_elements_) {
    if (IsLoudspeakerLayoutType(audio_element.GetType()) ||
        IsObjectType(audio_element.GetType())) {
      for (size_t i = 0; i < audio_element.GetNumberOfInputChannels(); ++i) {
        source_channel_indices.push_back(audio_element.GetFirstChannelIndex() +
                                         i);
      }
    }
  }
  return source_channel_indices;
}

absl::Status ObrImpl::UpdateAmbisonicEncoder() {
  // Check if Ambisonic Encoder is initialized, so it can be called.
  if (ambisonic_encoder_ != nullptr) {
    // Iterate over audio elements to look for loudspeaker channels/objects
    // which need to be encoded to Ambisonics. Update Ambisonic encoder
    // settings.
    size_t ambisonic_encoder_input_channel_index = 0;
    for (auto audio_element : audio_elements_) {
      // Iterate over loudspeaker channels.
      for (const auto& source : audio_element.GetLoudspeakerChannels()) {
        ambisonic_encoder_->SetSource(
            static_cast<int>(ambisonic_encoder_input_channel_index), 1.0f,
            source.GetAzimuth(), source.GetElevation(), source.GetDistance());
        ambisonic_encoder_input_channel_index++;
      }

      // Iterate over object input channels.
      for (const auto& source : audio_element.GetObjectChannels()) {
        ambisonic_encoder_->SetSource(
            static_cast<int>(ambisonic_encoder_input_channel_index), 1.0f,
            source.GetAzimuth(), source.GetElevation(), source.GetDistance());
        ambisonic_encoder_input_channel_index++;
      }
    }
    return absl::OkStatus();
  }
  return absl::FailedPreconditionError("Ambisonic encoder not initialized.");
}

size_t ObrImpl::GetNumberOfInputChannels() {
  size_t number_of_input_channels = 0;
  for (const auto& audio_element : audio_elements_) {
    number_of_input_channels += audio_element.GetNumberOfInputChannels();
  };
  return number_of_input_channels;
}

size_t ObrImpl::GetNumberOfAudioElements() { return audio_elements_.size(); }

absl::Status ObrImpl::AddAudioElement(const AudioElementType type) {
  // Impose a restriction that all audio elements must be of the same type.
  // This check will be removed in the future to allow for rendering of
  // different types of audio elements at the same pass of the binaural
  // Ambisonic decoder.
  if (!audio_elements_.empty() && audio_elements_.back().GetType() != type) {
    LOG(ERROR) << "Rendering only the same type of Audio Elements is supported."
               << " Remove the existing Audio Element before adding a new one.";
    return absl::FailedPreconditionError(
        "Only same-typed audio elements are supported.");
  }

  // Create an audio element configuration.
  auto audio_element_config = AudioElementConfig(type);

  if (!audio_elements_.empty()) {
    // If there are already audio elements, alter the first channel in the new
    // Audio Element config to shift the new element down on the input list.
    audio_element_config.SetFirstChannelIndex(
        audio_elements_.back().GetFirstChannelIndex() +
        audio_elements_.back().GetNumberOfInputChannels());
  }

  // Check if there is enough input channels.
  if (GetNumberOfInputChannels() +
          audio_element_config.GetNumberOfInputChannels() >
      kMaxSupportedNumInputChannels) {
    LOG(ERROR) << "More input channels requested than supported ("
               << kMaxSupportedNumInputChannels << ").";
    return absl::ResourceExhaustedError(
        "More input channels requested than supported.");
  }

  // All checks passed, add the Audio Element config.
  audio_elements_.push_back(audio_element_config);
  LOG(INFO) << "Added audio element: "
            << GetAudioElementTypeStr(audio_elements_.back().GetType()) << ".";

  RETURN_IF_NOT_OK(InitializeDsp());

  return absl::OkStatus();
}

absl::Status ObrImpl::RemoveLastAudioElement() {
  if (audio_elements_.empty()) {
    LOG(INFO) << "No audio elements to remove.";
    return absl::FailedPreconditionError("No audio elements to remove.");
  }

  LOG(INFO) << "Removing audio element: "
            << GetAudioElementTypeStr(audio_elements_.back().GetType()) << ".";
  audio_elements_.pop_back();

  if (audio_elements_.empty()) {
    LOG(INFO) << "No audio elements left.";
    return absl::OkStatus();
  }

  RETURN_IF_NOT_OK(InitializeDsp());

  RETURN_IF_NOT_OK(UpdateAmbisonicEncoder());

  return absl::OkStatus();
}

absl::Status ObrImpl::UpdateObjectPosition(size_t audio_element_index,
                                           float azimuth, float elevation,
                                           float distance) {
  // Check if the audio element index is valid.
  if (audio_element_index >= audio_elements_.size()) {
    LOG(ERROR) << "Invalid audio element index.";
    return absl::InvalidArgumentError("Invalid audio element index.");
  }

  if (audio_elements_[audio_element_index].GetObjectChannels().empty()) {
    LOG(ERROR) << "No objects in the audio element.";
    return absl::FailedPreconditionError("No objects in the audio element.");
  }

  // Get object channels from the element. For now if there are multiple input
  // channels in an object, configure them to use the same coordinates.
  for (auto& object_ch :
       audio_elements_[audio_element_index].GetObjectChannels()) {
    // Update the object position.
    object_ch.SetAzimuth(azimuth);
    object_ch.SetElevation(elevation);
    object_ch.SetDistance(distance);
  }

  RETURN_IF_NOT_OK(UpdateAmbisonicEncoder());

  return absl::OkStatus();
}

void ObrImpl::EnableHeadTracking(bool enable_head_tracking) {
  head_tracking_enabled_ = enable_head_tracking;
}

absl::Status ObrImpl::SetHeadRotation(float w, float x, float y, float z) {
  world_rotation_ = WorldRotation(w, x, y, z);

  return absl::OkStatus();
}

std::string ObrImpl::GetAudioElementConfigLogMessage() {
  /*!\brief Returns a padded string with a number formatted to two decimal
   * places.
   *
   * \param number Number to format.
   * \return Formatted number.
   */
  auto AEDValueToString = [](float number) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << number;
    std::string result = oss.str();
    if (result.size() < 7) {
      result = std::string(7 - result.size(), ' ') + result;
    }
    return result;
  };

  /*!\brief Returns a string padded with spaces to the left.
   *
   * \param str String to pad.
   * \param width Width of the padded string.
   * \return Padded string.
   */
  auto PadWithSpaces = [](std::string str, size_t width) {
    if (str.size() < width) {
      return std::string(width - str.size(), ' ') + str;
    }
    return str;
  };

  /*!\brief Renders a row of the audio element configuration table.
   *
   * \param element_data Data for the audio element.
   * \param channel_data Data for the channel.
   * \return Rendered row.
   */
  auto RenderRow = [](const std::string& element_data,
                      const std::string& channel_data) {
    std::string row = "|";
    row += element_data;
    row += "|";
    row += channel_data;
    row += "|\n";
    return row;
  };

  /*!\brief Converts a boolean value to a 'Yes' or 'No' string.
   *
   * \param value Boolean value to convert.
   * \return 'Yes' if true, 'No' if false.
   */
  auto BoolToYesNo = [](bool value) { return value ? "Yes" : "No"; };

  // Define the header.
  // <string, size_t> pairs are used to define the width of each column.
  std::vector<std::pair<std::string, size_t>> header = {// Audio Element data
                                                        {"AE ID", 5},
                                                        {"Type", 15},
                                                        {"BinFlt xOA", 10},
                                                        // Channel data
                                                        {"Ch ID", 5},
                                                        {"Ch Label", 10},
                                                        {"Azimuth", 10},
                                                        {"Elevation", 10},
                                                        {"Distance", 10},
                                                        {"LFE", 5}};
  // Render top border.
  std::string log_message = "+";
  for (auto& column : header) {
    log_message += std::string(column.second, '-');
    log_message += "+";
  }
  log_message += "\n";

  // Render header.
  log_message += "|";
  for (auto& column : header) {
    log_message += column.first;
    if (column.first.size() < column.second) {
      log_message += std::string(column.second - column.first.size(), ' ');
    }
    log_message += "|";
  }
  log_message += "\n";

  // Iterate over audio elements.
  std::string element_data;
  for (auto& audio_element : audio_elements_) {
    // Render middle border.
    log_message += "+";
    for (auto& column : header) {
      log_message += std::string(column.second, '-');
      log_message += "+";
    }
    log_message += "\n";

    auto type_string = GetAudioElementTypeStr(audio_element.GetType());
    if (!type_string.ok()) {
      LOG(ERROR) << "Failed to get audio element type string.";
      return "";
    }

    element_data =
        PadWithSpaces(std::to_string(&audio_element - &audio_elements_[0]),
                      header[0].second) +
        "|" +
        PadWithSpaces(std::string(type_string.value()), header[1].second) +
        "|" +
        PadWithSpaces(
            std::to_string(audio_element.GetBinauralFiltersAmbisonicOrder()),
            header[2].second);

    std::string channel_data;

    // Render Ambisonic channels.
    for (auto& channel : audio_element.GetAmbisonicChannels()) {
      channel_data = PadWithSpaces(std::to_string(channel.GetChannelIndex()),
                                   header[3].second) +
                     "|" + PadWithSpaces(channel.GetID(), header[4].second) +
                     "|" + PadWithSpaces("N/A", header[5].second) + "|" +
                     PadWithSpaces("N/A", header[6].second) + "|" +
                     PadWithSpaces("N/A", header[7].second) + "|" +
                     PadWithSpaces("N/A", header[8].second);

      log_message += RenderRow(element_data, channel_data);
    }

    // Render loudspeaker channels.
    for (auto& channel : audio_element.GetLoudspeakerChannels()) {
      channel_data =
          PadWithSpaces(std::to_string(channel.GetChannelIndex()),
                        header[3].second) +
          "|" + PadWithSpaces(channel.GetID(), header[4].second) + "|" +
          PadWithSpaces(AEDValueToString(channel.GetAzimuth()),
                        header[5].second) +
          "|" +
          PadWithSpaces(AEDValueToString(channel.GetElevation()),
                        header[6].second) +
          "|" +
          PadWithSpaces(AEDValueToString(channel.GetDistance()),
                        header[7].second) +
          "|" +
          PadWithSpaces(BoolToYesNo(channel.GetIsLFE()), header[8].second);

      log_message += RenderRow(element_data, channel_data);
    }

    // Render object channels.
    for (auto& channel : audio_element.GetObjectChannels()) {
      channel_data = PadWithSpaces(std::to_string(channel.GetChannelIndex()),
                                   header[3].second) +
                     "|" + PadWithSpaces(channel.GetID(), header[4].second) +
                     "|" +
                     PadWithSpaces(AEDValueToString(channel.GetAzimuth()),
                                   header[5].second) +
                     "|" +
                     PadWithSpaces(AEDValueToString(channel.GetElevation()),
                                   header[6].second) +
                     "|" +
                     PadWithSpaces(AEDValueToString(channel.GetDistance()),
                                   header[7].second) +
                     "|" + PadWithSpaces("N/A", header[8].second);

      log_message += RenderRow(element_data, channel_data);
    }
  }

  // Render bottom border.
  log_message += "+";
  for (auto& column : header) {
    log_message += std::string(column.second, '-');
    log_message += "+";
  }
  log_message += "\n";

  return log_message;
}

}  // namespace obr
