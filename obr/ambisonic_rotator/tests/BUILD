package(default_visibility = ["//src:__subpackages__"])

cc_test(
    name = "ambisonic_rotator_test",
    srcs = [
        "ambisonic_rotator_test.cc",
    ],
    deps = [
        "//obr/ambisonic_binaural_decoder:planar_interleaved_conversion",
        "//obr/ambisonic_encoder",
        "//obr/ambisonic_rotator",
        "//obr/ambisonic_rotator/tests:spherical_angle",
        "//obr/common",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "spherical_angle_test",
    srcs = [
        "spherical_angle_test.cc",
    ],
    deps = [
        "//obr/ambisonic_rotator/tests:spherical_angle",
        "@com_google_googletest//:gtest_main",
        "@eigen",
    ],
)

cc_library(
    name = "spherical_angle",
    srcs = [
        "spherical_angle.cc",
    ],
    hdrs = ["spherical_angle.h"],
    deps = [
        "//obr/common",
    ],
)
