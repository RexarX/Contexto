cmake_minimum_required(VERSION 3.20.0)

set(PROJECT_NAME Contexto)
project(${PROJECT_NAME}
    VERSION 0.1
    HOMEPAGE_URL https://github.com/RexarX/Contexto
)

set(CMAKE_CONFIGURATION_TYPES
    Debug
    RelWithDebInfo
    Release
    CACHE STRING "" FORCE
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

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

set(USERVER_FEATURE_PATCH_LIBPQ OFF CACHE BOOL "Patch libpq library to work with Postgres drivers")
set(USERVER_FEATURE_POSTGRESQL ON CACHE BOOL "Enable userver PostgreSQL support")

if(ENABLE_UNITY_BUILD)
    set(UNITY_BUILD_BATCH_SIZE 10 CACHE STRING "Number of source files per Unity batch")
endif()

add_subdirectory(third-party)
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
