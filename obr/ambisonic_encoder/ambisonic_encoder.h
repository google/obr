/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */
#ifndef OBR_AMBISONIC_ENCODER_AMBISONIC_ENCODER_H_
#define OBR_AMBISONIC_ENCODER_AMBISONIC_ENCODER_H_

#include <cstddef>
#include <vector>

#include "Eigen/Core"
#include "Eigen/Dense"
#include "absl/container/flat_hash_map.h"
#include "obr/ambisonic_encoder/associated_legendre_polynomials_generator.h"
#include "obr/audio_buffer/audio_buffer.h"

namespace obr {

class AmbisonicEncoder {
 public:
  /*!\brief Ambisonic Encoder constructor.
   *
   * \param number_of_input_channels Number of input channels (determines max
   *        number of sources to be processed).
   * \param ambisonic_order Ambisonic order (determines the number of output
   *        channels).
   */
  AmbisonicEncoder(size_t number_of_input_channels, size_t ambisonic_order);

  /*!\brief Default destructor. */
  ~AmbisonicEncoder() = default;

  /*!\brief Sets the parameters of a single source.
   *
   * \param input_channel Sets the input channel (0-indexed) associated with
   *        the source.
   * \param gain Sets the linear gain (0.5 = -6dB) applied to the source signal
   *        before encoding to Ambisonics. Independent of distance parameter.
   * \param azimuth Expressed in degrees (0 = front, 90 = left, 180 = back, -90
   *        = right).
   * \param elevation Expressed in degrees (0 = horizontal, 90 = up, -90 =
   *        down).
   * \param distance Expressed in meters (will impact final gain, not time
   *        delay).
   */
  void SetSource(size_t input_channel, float gain, float azimuth,
                 float elevation, float distance);

  /*!\brief Removes a source from the list of sources.
   *
   * The associated input channels are thereby muted.
   *
   * \param input_channel Determines which source to remove. (0-indexed)
   */
  void RemoveSource(size_t input_channel);

  /*!\brief Processing callback for planar audio data (AudioBuffer).
   *
   *
   * \param input_buffer Input buffer of samples.
   * \param output_buffer Output buffer of processed samples.
   */
  void ProcessPlanarAudioData(const AudioBuffer& input_buffer,
                              AudioBuffer* output_buffer) const;

 private:
  // Struct containing properties of a single source.
  struct SourceProperties {
    float gain;
    float azimuth;
    float elevation;
    float distance;
  };

  /*!\brief Calculates the spherical harmonic coefficients.
   *
   * Spherical harmonic coefficients are calculated for the given azimuth
   * and elevation. ACN/SN3D.
   *
   * \param azimuth Azimuth in degrees.
   * \param elevation Elevation in degrees.
   * \param ambisonic_order Ambisonic order.
   * \param coeffs Vector to store the calculated coefficients.
   */
  void GetShCoeffs(float azimuth, float elevation, size_t ambisonic_order,
                   std::vector<float>& coeffs);

  //  const size_t buffer_size_per_channel_;
  const size_t number_of_input_channels_;
  const size_t number_of_output_channels_;
  const size_t ambisonic_order_;

  // Map of structs containing the properties of each source.
  absl::flat_hash_map<size_t, SourceProperties> sources_;

  AssociatedLegendrePolynomialsGenerator alp_generator_;
  Eigen::MatrixXf encoding_matrix_;
};

}  // namespace obr

#endif  // OBR_AMBISONIC_ENCODER_AMBISONIC_ENCODER_H_
