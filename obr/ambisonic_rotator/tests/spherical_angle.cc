/*
* Copyright (c) 2025 Google LLC
*
* This source code is subject to the terms of the BSD 3-Clause Clear License,
* which you can find in the LICENSE file, and the Open Binaural Renderer
* Patent License 1.0, which you can find in the PATENTS file.
*/

#include "spherical_angle.h"

#include <cmath>

namespace obr {

SphericalAngle::SphericalAngle(float azimuth, float elevation)
    : azimuth_(azimuth), elevation_(elevation) {}

SphericalAngle::SphericalAngle() : SphericalAngle(0.0f, 0.0f) {}

SphericalAngle::SphericalAngle(const SphericalAngle& other)
    : azimuth_(other.azimuth_), elevation_(other.elevation_) {}

SphericalAngle& SphericalAngle::operator=(const SphericalAngle other) {
  if (&other == this) {
    return *this;
  }
  this->azimuth_ = other.azimuth_;
  this->elevation_ = other.elevation_;
  return *this;
}

SphericalAngle SphericalAngle::FromWorldPosition(
    const WorldPosition& world_position) {
  return SphericalAngle(
      std::atan2(-world_position[0], -world_position[2]),
      std::atan2(world_position[1],
                 std::sqrt(world_position[0] * world_position[0] +
                           world_position[2] * world_position[2])));
}

SphericalAngle SphericalAngle::FromDegrees(float azimuth_degrees,
                                           float elevation_degrees) {
  return SphericalAngle(azimuth_degrees * kRadiansFromDegrees,
                        elevation_degrees * kRadiansFromDegrees);
}

SphericalAngle SphericalAngle::FlipAzimuth() const {
  return SphericalAngle(-azimuth_, elevation_);
}

WorldPosition SphericalAngle::GetWorldPositionOnUnitSphere() const {
  return WorldPosition(-std::cos(elevation_) * std::sin(azimuth_),
                       std::sin(elevation_),
                       -std::cos(elevation_) * std::cos(azimuth_));
}

SphericalAngle SphericalAngle::Rotate(const WorldRotation& rotation) const {
  const WorldPosition original_world_position = GetWorldPositionOnUnitSphere();
  const WorldPosition rotated_world_position =
      rotation * original_world_position;
  return FromWorldPosition(rotated_world_position);
}

bool SphericalAngle::operator==(const SphericalAngle& other) const {
  return (azimuth_ == other.azimuth_) && (elevation_ == other.elevation_);
}

}  // namespace obr
