add_executable(utf8_test utf8_test.cpp)

set_target_properties(utf8_test PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} UPOUTPUTCONFIG)
    set_target_properties(utf8_test PROPERTIES
        TARGET_NAME_${UPOUTPUTCONFIG} utf8_test
        ARCHIVE_OUTPUT_NAME_${UPOUTPUTCONFIG} utf8_test
        RUNTIME_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
        LIBRARY_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
        ARCHIVE_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
    )
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

target_compile_options(utf8_test PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:RelWithDebInfo>:-O3 -flto>
        $<$<CONFIG:Release>:-O3 -flto>
        -fPIC
    >
)

target_precompile_headers(utf8_test PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/pch.hpp>
)

target_include_directories(utf8_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(utf8_test PRIVATE
    userver-utest
    ${PROJECT_NAME}_objs
)

add_google_tests(utf8_test)
