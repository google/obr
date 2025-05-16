/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */

#ifndef BINAURAL_FILTERS_WRAPPER_H_
#define BINAURAL_FILTERS_WRAPPER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "binaural_filters_1_oa_l.h"
#include "binaural_filters_1_oa_r.h"
#include "binaural_filters_2_oa_l.h"
#include "binaural_filters_2_oa_r.h"
#include "binaural_filters_3_oa_l.h"
#include "binaural_filters_3_oa_r.h"
#include "binaural_filters_4_oa_l.h"
#include "binaural_filters_4_oa_r.h"
#include "binaural_filters_5_oa_l.h"
#include "binaural_filters_5_oa_r.h"
#include "binaural_filters_6_oa_l.h"
#include "binaural_filters_6_oa_r.h"
#include "binaural_filters_7_oa_l.h"
#include "binaural_filters_7_oa_r.h"

namespace obr {

class BinauralFiltersWrapper {
 public:
  BinauralFiltersWrapper();
  ~BinauralFiltersWrapper();

  std::unique_ptr<std::string> GetFile(const std::string& filename) const;

 private:
  using AssetMap = std::unordered_map<std::string, std::function<std::vector<unsigned char>()>>;
  static const AssetMap kAssetMap;
};

}  // namespace obr

#endif  // BINAURAL_FILTERS_WRAPPER_H_
