package(default_visibility = ["//obr/cli/tests:__subpackages__"])

cc_test(
    name = "obr_cli_lib_test",
    srcs = ["obr_cli_lib_test.cc"],
    data = [
        "//obr/cli/testdata:input_wav_files",
        "//obr/cli/testdata:textprotos",
    ],
    shard_count = 4,
    deps = [
        "//obr/cli:obr_cli_lib",
        "//obr/renderer:audio_element_type",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
