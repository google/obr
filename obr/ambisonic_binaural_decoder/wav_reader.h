/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */
// TODO(b/400661672): Use a common library to handle WAV files.
#ifndef OBR_WAV_READER_H_
#define OBR_WAV_READER_H_

#include <cstddef>
#include <cstdint>
#include <istream>

// This code is forked from Resonance Audio's `wav_reader.h`.
namespace obr {

/*!\brief Basic RIFF WAVE decoder that supports multichannel 16-bit PCM. */
class WavReader {
 public:
  /*!\brief Constructor decodes WAV header.
   *
   * \param binary_stream Binary input stream to read from.
   */
  explicit WavReader(std::istream* binary_stream);

  // True if WAV header was successfully parsed.
  bool IsHeaderValid() const;

  // Returns the total number of samples defined in the WAV header. Note that
  // the actual number of samples in the file can differ.
  size_t GetNumTotalSamples() const;

  // Returns number of channels.
  size_t GetNumChannels() const;

  // Returns sample rate in Hertz.
  int GetSampleRateHz() const;

  /*!\brief Seek to a specific frame position within the wave file. If
   * frame_position is not a valid address, then the internal read position
   * remains unchanged.
   *
   * \param frame_position Destination frame position for play cursor.
   * \return Actual frame position of cursor after this seek operation. A
   *         negative return value indicates a stream failure.
   */
  int64_t SeekToFrame(const uint64_t frame_position);

  /*!\brief Reads samples from WAV file into target buffer.
   *
   * \param num_samples Number of samples to read.
   * \param target_buffer Target buffer to write to.
   * \return Number of decoded samples.
   */
  size_t ReadSamples(size_t num_samples, int16_t* target_buffer);

 private:
  /*!\brief Parses WAV header.
   *
   * \return True on success.
   */
  bool ParseHeader();

  /*!\brief Helper method to read binary data from input stream.
   *
   * \param size Number of bytes to read.
   * \param target_ptr Target buffer to write to.
   * \return Number of bytes read.
   */
  size_t ReadBinaryDataFromStream(void* target_ptr, size_t size);

  // Binary input stream.
  std::istream* binary_stream_;

  // Flag indicating if the WAV header was parsed successfully.
  bool init_;

  // Number of audio channels.
  size_t num_channels_;

  // Sample rate in Hertz.
  int sample_rate_hz_;

  // Total number of samples.
  size_t num_total_samples_;

  // Number of remaining samples in WAV file.
  size_t num_remaining_samples_;

  // Bytes per sample as defined in the WAV header.
  size_t bytes_per_sample_;

  // Offset into data stream where PCM data begins.
  uint64_t pcm_offset_bytes_;
};

}  // namespace obr

#endif  // OBR_WAV_READER_H_
