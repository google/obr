package(default_visibility = ["//obr:__subpackages__"])

cc_library(
    name = "audio_element_config",
    srcs = ["audio_element_config.cc"],
    hdrs = ["audio_element_config.h"],
    deps = [
        ":audio_element_type",
        ":input_channel_config",
        ":loudspeaker_layouts",
        "//obr/common",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "audio_element_type",
    hdrs = ["audio_element_type.h"],
    deps = [
        "@com_google_absl//absl/base:no_destructor",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "obr_impl",
    srcs = [
        "obr_impl.cc",
    ],
    hdrs = ["obr_impl.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":audio_element_config",
        ":audio_element_type",
        "//obr/ambisonic_binaural_decoder",
        "//obr/ambisonic_binaural_decoder:fft_manager",
        "//obr/ambisonic_binaural_decoder:resampler",
        "//obr/ambisonic_binaural_decoder:sh_hrir_creator",
        "//obr/ambisonic_encoder",
        "//obr/ambisonic_rotator",
        "//obr/audio_buffer",
        "//obr/common",
        "//obr/peak_limiter",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/synchronization",
    ],
)

cc_library(
    name = "input_channel_config",
    hdrs = ["input_channel_config.h"],
    deps = [],
)

cc_library(
    name = "loudspeaker_layouts",
    hdrs = ["loudspeaker_layouts.h"],
    deps = [
        ":audio_element_type",
        ":input_channel_config",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)
