cmake_minimum_required(VERSION 3.28)
project(obr_cmake)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    # Disable type conversion warnings.
    add_definitions(/wd4244)
    add_definitions(/wd4305)
    add_definitions(/wd4722)
endif ()

include(FetchContent)

# Fetch Abseil library
include(FetchContent)
FetchContent_Declare(
        absl
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 4447c7562e3bc702ade25105912dce503f0c4010
)
FetchContent_MakeAvailable(absl)

# Fetch eigen library
include(FetchContent)
FetchContent_Declare(
        eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3147391d946bb4b6c68edd901f2add6ac1f31f8c
)
FetchContent_MakeAvailable(eigen)

# TODO(b/407470208): Fetch googletest.

# Fetch PFFFT.
FetchContent_Declare(
        pffft
        GIT_REPOSITORY https://bitbucket.org/jpommier/pffft.git
        GIT_TAG 180c2d62717c0150d8aebd165fce19ee4e5f86f9
)
FetchContent_MakeAvailable(pffft)

# Manually add PFFFT library.
add_library(pffft STATIC
        ${pffft_SOURCE_DIR}/pffft.c
)

# Collect all source files.
file(GLOB SourceFiles CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_binaural_decoder/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_binaural_decoder/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_binaural_decoder/binaural_filters/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_binaural_decoder/binaural_filters/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_encoder/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_encoder/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_rotator/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/ambisonic_rotator/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/audio_buffer/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/audio_buffer/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/common/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/common/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/peak_limiter/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/peak_limiter/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/renderer/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/renderer/*.cc"

)

# Exclude the following files / directories.
list(REMOVE_ITEM SourceFiles
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/common/test_util.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/obr/common/test_util.cc"
)


# Define obr library target.
add_library(obr STATIC)

# Add the collected source files to the library target.
target_sources(obr PRIVATE ${SourceFiles})

# Set include directories.
target_include_directories(obr PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${eigen_SOURCE_DIR}
        ${absl_SOURCE_DIR}
        ${pffft_SOURCE_DIR}
)

# Link libraries.
target_link_libraries(obr PRIVATE
        pffft
        eigen
        absl::base
        absl::die_if_null
        absl::flat_hash_map
        absl::log
        absl::log_internal_message
        absl::log_internal_check_op
        absl::status
        absl::statusor
        absl::strings
)

# TODO(b/407470208): Define obr tests.

# TODO(b/407470208): Define obr_cli executable.
