file(GLOB_RECURSE OBJ_SOURCES
    contexto/*.cpp
    contexto/models/*.cpp
)

if(ENABLE_UNITY_BUILD)
    set_source_files_properties(${OBJ_SOURCES} PROPERTIES UNITY_GROUP OBJ_SOURCES)
    if(CMAKE_PCH_ENABLED)
        set_source_files_properties(pch.hpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
    endif()
endif()

add_library(${PROJECT_NAME}_objs OBJECT ${OBJ_SOURCES})

set_target_properties(${PROJECT_NAME}_objs PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(ENABLE_UNITY_BUILD)
    set_target_properties(${PROJECT_NAME}_objs PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_MODE BATCH
        CMAKE_UNITY_BUILD_BATCH_SIZE ${UNITY_BUILD_BATCH_SIZE}
    )
endif()

target_precompile_headers(${PROJECT_NAME}_objs PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:pch.hpp>
)

find_package(Eigen3 REQUIRED)

target_link_libraries(${PROJECT_NAME}_objs PRIVATE
    userver-core
    Eigen3::Eigen
)

target_include_directories(${PROJECT_NAME}_objs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)

target_compile_definitions(${PROJECT_NAME}_objs PRIVATE
    $<$<CONFIG:Debug>:
        DEBUG_MODE
    >
    $<$<CONFIG:RelWithDebInfo>:
        RELEASE_WITH_DEDUG_INFO_MODE
    >
    $<$<CONFIG:Release>:
        RELEASE_MODE
    >
)

target_compile_definitions(${PROJECT_NAME}_objs PRIVATE
    $<$<BOOL:ENABLE_PROFILING>:ENABLE_PROFILING>
)

target_compile_options(${PROJECT_NAME}_objs PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:RelWithDebInfo>:-O3 -flto>
        $<$<CONFIG:Release>:-O3 -flto>
        -fPIC
    >
)

if(ENABLE_SIMD_SSE2)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse2>
            $<$<CONFIG:Release>:-msse2>
        >
    )
elseif(ENABLE_SIMD_SSE3)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse3>
            $<$<CONFIG:Release>:-msse3>
        >
    )
elseif(ENABLE_SIMD_SSSE3)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mssse3>
            $<$<CONFIG:Release>:-mssse3>
        >
 )
elseif(ENABLE_SIMD_SSE4_1)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse4.1>
            $<$<CONFIG:Release>:-msse4.1>
        >
 )
elseif(ENABLE_SIMD_SSE4_2)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:Release>:-msse4.2>
            $<$<CONFIG:RelWithDebInfo>:-msse4.2>
        >
 )
elseif(ENABLE_SIMD_AVX)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mavx>
            $<$<CONFIG:Release>:-mavx>
        >
    )
elseif(ENABLE_SIMD_AVX2)
    target_compile_options(${PROJECT_NAME}_objs PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mavx2>
            $<$<CONFIG:Release>:-mavx2>
        >
    )
endif()

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:pch.hpp>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    userver-core
    Eigen3::Eigen
    ${PROJECT_NAME}_objs
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:
        DEBUG_MODE
    >
    $<$<CONFIG:RelWithDebInfo>:
        RELEASE_WITH_DEDUG_INFO_MODE
    >
    $<$<CONFIG:Release>:
        RELEASE_MODE
    >
)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} UPOUTPUTCONFIG)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_${UPOUTPUTCONFIG} ${PROJECT_NAME}
        ARCHIVE_OUTPUT_NAME_${UPOUTPUTCONFIG} ${PROJECT_NAME}
        RUNTIME_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}
        LIBRARY_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}
        ARCHIVE_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}
    )
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

set(CONFIG_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml
)

install(
    FILES ${CONFIG_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)

set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../assets)
if(EXISTS ${ASSETS_DIR})
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        if(OUTPUTCONFIG STREQUAL "Release")
            # Copy assets for Release builds
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${ASSETS_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}/assets
                COMMENT "Copying assets directory to ${OUTPUTCONFIG} build output"
            )
        else()
            # Create symlink for other builds
            add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}
                COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${ASSETS_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${OUTPUTCONFIG}/assets
                COMMENT "Creating symlink to assets directory for ${OUTPUTCONFIG} build output"
            )
        endif()
    endforeach()

    # Install assets with the project
    install(
        DIRECTORY ${ASSETS_DIR}/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/assets
        COMPONENT ${PROJECT_NAME}
    )

    message(STATUS "Assets directory will be included in build output (symlinks for debug builds, copies for release)")
else()
    message(STATUS "Assets directory not found at ${ASSETS_DIR}")
endif()
