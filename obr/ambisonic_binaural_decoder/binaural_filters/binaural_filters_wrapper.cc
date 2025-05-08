/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */

#include "binaural_filters_wrapper.h"

namespace obr {

const BinauralFiltersWrapper::AssetMap BinauralFiltersWrapper::kAssetMap = {
  {"1OA_L", filter_files::BinauralFilters1OAL},
  {"1OA_R", filter_files::BinauralFilters1OAR},
  {"2OA_L", filter_files::BinauralFilters2OAL},
  {"2OA_R", filter_files::BinauralFilters2OAR},
  {"3OA_L", filter_files::BinauralFilters3OAL},
  {"3OA_R", filter_files::BinauralFilters3OAR},
  {"4OA_L", filter_files::BinauralFilters4OAL},
  {"4OA_R", filter_files::BinauralFilters4OAR},
  {"5OA_L", filter_files::BinauralFilters5OAL},
  {"5OA_R", filter_files::BinauralFilters5OAR},
  {"6OA_L", filter_files::BinauralFilters6OAL},
  {"6OA_R", filter_files::BinauralFilters6OAR},
  {"7OA_L", filter_files::BinauralFilters7OAL},
  {"7OA_R", filter_files::BinauralFilters7OAR},
};

BinauralFiltersWrapper::BinauralFiltersWrapper() {}

BinauralFiltersWrapper::~BinauralFiltersWrapper() {}

std::unique_ptr<std::string> BinauralFiltersWrapper::GetFile(const std::string& filename) const {
  auto it = kAssetMap.find(filename);
  if (it == kAssetMap.end()) {
    return nullptr;
  }
  auto data = it->second();
  return std::make_unique<std::string>(reinterpret_cast<const char*>(data.data()), data.size());
}

}  // namespace obr
