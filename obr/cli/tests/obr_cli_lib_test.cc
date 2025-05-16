/*
 * Copyright (c) 2025 Google LLC
 *
 * This source code is subject to the terms of the BSD 3-Clause Clear License,
 * which you can find in the LICENSE file, and the Open Binaural Renderer
 * Patent License 1.0, which you can find in the PATENTS file.
 */
#include "obr/cli/obr_cli_lib.h"

#include <cstddef>
#include <filesystem>
#include <string>

// [internal] Placeholder for get runfiles header.
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "gtest/gtest.h"
#include "obr/renderer/audio_element_type.h"

namespace obr {
namespace {

const std::filesystem::path kTestDataDir = std::filesystem::path("obr") / "cli" / "testdata";
constexpr absl::string_view kNoObaMetadata = "";
constexpr size_t kBufferSize = 256;

std::string GetAndCleanupOutputFileName(absl::string_view suffix) {
  const testing::TestInfo* const test_info =
      testing::UnitTest::GetInstance()->current_test_info();
  std::string filename = absl::StrCat(test_info->name(), "-",
                                      test_info->test_suite_name(), suffix);

  // It is possible that the test case name contain the '/' character.
  // Replace it with '-' to form a legal file name.
  absl::StrReplaceAll({{"/", "-"}}, &filename);
  const std::filesystem::path test_specific_filename =
      std::filesystem::path(::testing::TempDir()) / filename;

  std::filesystem::remove(test_specific_filename);
  return test_specific_filename.string();
}

struct CliTestCase {
  AudioElementType input_type;
  absl::string_view wav_filename;
  absl::string_view oba_metadata_filename = "";
  bool expected_ok = true;
};

using CliMainTest = ::testing::TestWithParam<CliTestCase>;

TEST_P(CliMainTest, RenderToFiles) {
  const auto& test_case = GetParam();
  const auto input_filename =
      (std::filesystem::current_path() / kTestDataDir / test_case.wav_filename)
          .string();
  const std::string oba_metadata_filename =
      test_case.oba_metadata_filename.empty()
          ? ""
          : (std::filesystem::current_path() / kTestDataDir /
             test_case.oba_metadata_filename)
                .string();

  const auto output_filename = GetAndCleanupOutputFileName(".wav");
  const auto status =
      obr::ObrCliMain(test_case.input_type, oba_metadata_filename,
                      input_filename, output_filename, kBufferSize);
  EXPECT_EQ(status.ok(), test_case.expected_ok);
  EXPECT_EQ(std::filesystem::exists(output_filename), test_case.expected_ok);
}

INSTANTIATE_TEST_SUITE_P(
    Succeeds, CliMainTest,
    ::testing::Values(CliTestCase{AudioElementType::k3OA, "7.1.4_test_3OA.wav"},
                      CliTestCase{AudioElementType::k7OA, "7.1.4_test_7OA.wav"},
                      CliTestCase{AudioElementType::kLayout7_1_4_ch,
                                  "7.1.4_test_individual_channels.wav"},
                      CliTestCase{AudioElementType::kObjectMono,
                                  "7.1.4_test_individual_channels.wav",
                                  "7.1.4_test_oba.textproto"}));

INSTANTIATE_TEST_SUITE_P(
    FailsWithMismatchingTypeAndInput, CliMainTest,
    ::testing::Values(CliTestCase{AudioElementType::k3OA, "7.1.4_test_7OA.wav",
                                  kNoObaMetadata, false},
                      CliTestCase{AudioElementType::k7OA,
                                  "7.1.4_test_individual_channels.wav",
                                  kNoObaMetadata, false},
                      CliTestCase{AudioElementType::kLayout7_1_4_ch,
                                  "7.1.4_test_3OA.wav", kNoObaMetadata, false},
                      CliTestCase{AudioElementType::kObjectMono,
                                  "7.1.4_test_3OA.wav",
                                  "7.1.4_test_oba.textproto", false},
                      CliTestCase{AudioElementType::kObjectMono,
                                  "7.1.4_test_individual_channels.wav",
                                  kNoObaMetadata, false}));

}  // namespace
}  // namespace obr
