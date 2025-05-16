/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */

#include "obr/ambisonic_binaural_decoder/sh_hrir_creator.h"

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>

#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "absl/log/log.h"
#include "obr/ambisonic_binaural_decoder/binaural_filters/binaural_filters_wrapper.h"
#include "obr/ambisonic_binaural_decoder/planar_interleaved_conversion.h"
#include "obr/ambisonic_binaural_decoder/resampler.h"
#include "obr/ambisonic_binaural_decoder/wav.h"
#include "obr/audio_buffer/audio_buffer.h"
#include "obr/common/ambisonic_utils.h"

namespace obr {

std::unique_ptr<AudioBuffer> CreateShHrirsFromWav(const Wav& wav,
                                                  int target_sample_rate_hz,
                                                  Resampler* resampler) {
  DCHECK_NE(resampler, nullptr);
  const size_t num_channels = wav.GetNumChannels();
  CHECK(IsValidAmbisonicOrder(num_channels));

  const size_t sh_hrir_length = wav.interleaved_samples().size() / num_channels;
  std::unique_ptr<AudioBuffer> sh_hrirs(
      new AudioBuffer(num_channels, sh_hrir_length));
  FillAudioBuffer(wav.interleaved_samples(), num_channels, sh_hrirs.get());

  const int wav_sample_rate_hz = wav.GetSampleRateHz();
  CHECK_GT(wav_sample_rate_hz, 0);
  CHECK_GT(target_sample_rate_hz, 0);
  if (wav_sample_rate_hz != target_sample_rate_hz) {
    if (!Resampler::AreSampleRatesSupported(wav_sample_rate_hz,
                                            target_sample_rate_hz)) {
      LOG(FATAL) << "Unsupported sampling rates for loading HRIRs: "
                 << wav_sample_rate_hz << ", " << target_sample_rate_hz;
    }
    resampler->ResetState();
    // Resample the SH-HRIRs if necessary.
    resampler->SetRateAndNumChannels(wav_sample_rate_hz, target_sample_rate_hz,
                                     num_channels);
    std::unique_ptr<AudioBuffer> resampled_sh_hrirs(new AudioBuffer(
        num_channels, resampler->GetNextOutputLength(sh_hrir_length)));
    resampler->Process(*sh_hrirs, resampled_sh_hrirs.get());
    return resampled_sh_hrirs;
  }
  return sh_hrirs;
}

std::unique_ptr<AudioBuffer> CreateShHrirsFromAssets(
    const std::string& filename, int target_sample_rate_hz,
    Resampler* resampler) {
  BinauralFiltersWrapper hrtf_assets;
  std::unique_ptr<std::string> sh_hrir_data = hrtf_assets.GetFile(filename);

  // Check if the asset was found.
  if (sh_hrir_data == nullptr) {
    LOG(ERROR) << "Could not find asset: " << filename;
  }

  std::istringstream wav_data_stream(*ABSL_DIE_IF_NULL(sh_hrir_data));
  std::unique_ptr<const Wav> wav = Wav::CreateOrNull(&wav_data_stream);
  return CreateShHrirsFromWav(*wav, target_sample_rate_hz, resampler);
}

}  // namespace obr
