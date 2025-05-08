package(default_visibility = ["//obr:__subpackages__"])

cc_library(
    name = "ambisonic_binaural_decoder",
    srcs = [
        "ambisonic_binaural_decoder.cc",
    ],
    hdrs = ["ambisonic_binaural_decoder.h"],
    deps = [
        ":fft_manager",
        ":partitioned_fft_filter",
        "//obr/audio_buffer",
        "//obr/common",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "dsp_utils",
    srcs = ["dsp_utils.cc"],
    hdrs = ["dsp_utils.h"],
    deps = [
        "//obr/audio_buffer",
        "//obr/common",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "fft_manager",
    srcs = ["fft_manager.cc"],
    hdrs = ["fft_manager.h"],
    deps = [
        "//obr/audio_buffer",
        "//obr/audio_buffer:simd_utils",
        "//obr/common",
        "@com_google_absl//absl/log:check",
        "@pffft",
    ],
)

cc_library(
    name = "partitioned_fft_filter",
    srcs = ["partitioned_fft_filter.cc"],
    hdrs = ["partitioned_fft_filter.h"],
    deps = [
        ":dsp_utils",
        ":fft_manager",
        "//obr/audio_buffer",
        "//obr/audio_buffer:simd_utils",
        "//obr/common",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "planar_interleaved_conversion",
    srcs = ["planar_interleaved_conversion.cc"],
    hdrs = ["planar_interleaved_conversion.h"],
    deps = [
        ":sample_type_conversion",
        "//obr/audio_buffer",
        "//obr/audio_buffer:simd_utils",
        "//obr/common",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "resampler",
    srcs = ["resampler.cc"],
    hdrs = ["resampler.h"],
    deps = [
        ":dsp_utils",
        "//obr/audio_buffer",
        "//obr/audio_buffer:simd_utils",
        "//obr/common",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "sample_type_conversion",
    srcs = ["sample_type_conversion.cc"],
    hdrs = ["sample_type_conversion.h"],
    deps = [
        "//obr/audio_buffer:simd_utils",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "sh_hrir_creator",
    srcs = ["sh_hrir_creator.cc"],
    hdrs = ["sh_hrir_creator.h"],
    deps = [
        ":planar_interleaved_conversion",
        ":resampler",
        ":wav",
        "//obr/ambisonic_binaural_decoder/binaural_filters:binaural_filters_wrapper",
        "//obr/audio_buffer",
        "//obr/common",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
    ],
)

cc_library(
    name = "wav",
    srcs = ["wav.cc"],
    hdrs = ["wav.h"],
    deps = [":wav_reader"],
)

cc_library(
    name = "wav_reader",
    srcs = ["wav_reader.cc"],
    hdrs = ["wav_reader.h"],
    deps = [
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:die_if_null",
    ],
)
