cmake_minimum_required(VERSION 3.25.0)

set(PROJECT_NAME VoiceAssistant)
project(${PROJECT_NAME})

set(CMAKE_CONFIGURATION_TYPES
    Debug
    RelWithDebInfo
    Release
    CACHE STRING "" FORCE
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENABLE_SIMD_SSE2 "Enable SSE2 optimizations" OFF)
option(ENABLE_SIMD_SSE3 "Enable SSE3 optimizations" OFF)
option(ENABLE_SIMD_SSSE3 "Enable SSSE3 optimizations" OFF)
option(ENABLE_SIMD_SSE4_1 "Enable SSE 4.1 optimizations" OFF)
option(ENABLE_SIMD_SSE4_2 "Enable SSE 4.2 optimizations" OFF)
option(ENABLE_SIMD_AVX "Enable AVX optimizations" OFF)
option(ENABLE_SIMD_AVX2 "Enable AVX2 optimizations" OFF)

option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_PROFILING "Enable profiling" OFF)
option(ENABLE_UNITY_BUILD "Enable Unity Build" OFF)

if(ENABLE_UNITY_BUILD)
    set(UNITY_BUILD_BATCH_SIZE 10 CACHE STRING "Number of source files per Unity batch")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(DownloadUserver)

find_package(userver COMPONENTS core QUIET)
if(NOT userver_FOUND)
    download_userver(TRY_DIR src/third_party/userver)
endif()

userver_setup_environment()

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

add_subdirectory(src)
add_subdirectory(src/third-party)

if(BUILD_TESTS)
    userver_testsuite_add_simple()
    add_subdirectory(tests)
endif()