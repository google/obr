/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */
#ifndef OBR_AMBISONIC_ENCODER_ASSOCIATED_LEGENDRE_POLYNOMIALS_GENERATOR_H_
#define OBR_AMBISONIC_ENCODER_ASSOCIATED_LEGENDRE_POLYNOMIALS_GENERATOR_H_

#include <cstddef>
#include <vector>

// This code is forked from Resonance Audio's
// `associated_legendre_polynomials_generator.h`.
namespace obr {

/*!\brief Generates associated Legendre polynomials. */
class AssociatedLegendrePolynomialsGenerator {
 public:
  /*!\brief Constructs a generator for associated Legendre polynomials (ALP).
   *
   * \param max_degree Maximum ALP degree supported by this generator.
   * \param condon_shortley_phase Whether the Condon-Shortley phase, (-1)^order,
   *        should be included in the polynomials generated.
   * \param compute_negative_order Whether this generator should compute
   *        negative-ordered polynomials.
   */
  AssociatedLegendrePolynomialsGenerator(int max_degree,
                                         bool condon_shortley_phase,
                                         bool compute_negative_order);

  /*!\brief Generates the associated Legendre polynomials for the given `x`.
   *
   * \param x Abscissa (the polynomials' variable).
   * \return Output vector of computed sequence values.
   */
  std::vector<float> Generate(float x) const;

  /*!\brief Gets the produced number of associated Legendre polynomials.
   *
   * \return Number of associated Legendre polynomials this generator
   *         produces.
   */
  size_t GetNumValues() const;

  /*!\brief Gets the index into the output vector, given `degree` and `order`.
   *
   * \param degree Polynomial's degree.
   * \param order Polynomial's order.
   * \return Index into the vector of computed values corresponding to the
   *         specified ALP.
   */
  size_t GetIndex(int degree, int order) const;

 private:
  /*!\brief Computes the ALP for (degree, order) the given `x`.
   *
   * ALP is computed using recurrence relations. It is assumed that the ALPs
   * necessary for each computation are already computed and stored in `values`.
   *
   * \param degree Degree of the polynomial being computed.
   * \param degree Order of the polynomial being computed.
   * \param values Previously computed values.
   * \return Computed polynomial.
   */
  inline float ComputeValue(int degree, int order, float x,
                            const std::vector<float>& values) const;

  /*!\brief Checks the validity of the given index.
   *
   * \param degree The polynomial's degree.
   * \param order The polynomial's order.
   */
  inline void CheckIndexValidity(int degree, int order) const;

  // The maximum polynomial degree that can be computed; must be >= 0.
  const int max_degree_;

  // Whether the Condon-Shortley phase, (-1)^order, should be included in the
  // polynomials generated.
  const bool condon_shortley_phase_;

  // Whether this generator should compute negative-ordered polynomials.
  const bool compute_negative_order_;
};

}  // namespace obr

#endif  // OBR_AMBISONIC_ENCODER_ASSOCIATED_LEGENDRE_POLYNOMIALS_GENERATOR_H_
