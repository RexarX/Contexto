add_executable(hello_test hello_test.cpp)

set_target_properties(hello_test PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} UPOUTPUTCONFIG)
    set_target_properties(hello_test PROPERTIES
        TARGET_NAME_${UPOUTPUTCONFIG} hello_test
        ARCHIVE_OUTPUT_NAME_${UPOUTPUTCONFIG} hello_test
        RUNTIME_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
        LIBRARY_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
        ARCHIVE_OUTPUT_DIRECTORY_${UPOUTPUTCONFIG}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/tests/${OUTPUTCONFIG}
    )
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

target_compile_options(hello_test PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:RelWithDebInfo>:-O3 -flto>
        $<$<CONFIG:Release>:-O3 -flto>
        -fPIC
    >
)

if(ENABLE_SIMD_SSE2)
    target_compile_options(hello_test PRIVATE
	    $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse2>
		    $<$<CONFIG:Release>:-msse2>   
	    >
    )
elseif(ENABLE_SIMD_SSE3)
    target_compile_options(hello_test PRIVATE
	    $<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse3>
		    $<$<CONFIG:Release>:-msse3> 
	    >
    )
elseif(ENABLE_SIMD_SSSE3)
	target_compile_options(hello_test PRIVATE
		$<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mssse3>
			$<$<CONFIG:Release>:-mssse3>
		>
	)
elseif(ENABLE_SIMD_SSE4_1)
	target_compile_options(hello_test PRIVATE
		$<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-msse4.1>
			$<$<CONFIG:Release>:-msse4.1>	
		>
	)
elseif(ENABLE_SIMD_SSE4_2)
	target_compile_options(hello_test PRIVATE
		$<$<CXX_COMPILER_ID:Clang,GNU>:
			$<$<CONFIG:Release>:-msse4.2>
			$<$<CONFIG:RelWithDebInfo>:-msse4.2>
		>
	)
elseif(ENABLE_SIMD_AVX)
    target_compile_options(hello_test PRIVATE
		$<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mavx>
			$<$<CONFIG:Release>:-mavx>
			
		>
    )
elseif(ENABLE_SIMD_AVX2)
    target_compile_options(hello_test PRIVATE
		$<$<CXX_COMPILER_ID:Clang,GNU>:
            $<$<CONFIG:RelWithDebInfo>:-mavx2>
			$<$<CONFIG:Release>:-mavx2>
		>
    )
endif()

target_precompile_headers(hello_test PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/../../src/pch.hpp>
)

target_include_directories(hello_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(hello_test PRIVATE
    userver::core
    userver::utest
    ${PROJECT_NAME}_objs
)

add_google_tests(hello_test)