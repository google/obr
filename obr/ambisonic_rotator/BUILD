package(default_visibility = ["//src:__subpackages__"])

cc_library(
    name = "ambisonic_rotator",
    srcs = [
        "ambisonic_rotator.cc",
    ],
    hdrs = ["ambisonic_rotator.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//obr/audio_buffer",
        "//obr/common",
        "@com_google_absl//absl/log:check",
        "@eigen",
    ],
)
