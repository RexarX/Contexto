set(USERVER_FEATURE_PATCH_LIBCURL OFF CACHE BOOL "")

add_subdirectory(userver)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

if(BUILD_TESTS)
    # We need to make sure the project target exists before adding tests
    # Delaying the testsuite setup until after the main targets are created
    set(USERVER_TESTSUITE_ENABLED TRUE CACHE BOOL "Enable testsuite")
endif()

# Function to add testsuite after the main targets are created
function(add_contexto_testsuite)
    if(NOT USERVER_TESTSUITE_ENABLED)
        return()
    endif()

    # Specify the path to the static config file for tests
    set(CONFIG_PATH "${CMAKE_SOURCE_DIR}/configs/static_config.yaml")

    # Make sure the path exists before trying to use it
    if(NOT EXISTS "${CONFIG_PATH}")
        message(WARNING "Test config file not found at ${CONFIG_PATH}")
        message(STATUS "Creating default test config...")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/configs")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/userver/testsuite/static_config.yaml.in"
            "${CONFIG_PATH}"
            @ONLY
        )
    endif()

    # Check if the target Contexto exists before adding testsuite
    if(TARGET ${PROJECT_NAME})
        # Now add the testsuite with the CONFIG_PATH parameter
        message(STATUS "Adding testsuite for ${PROJECT_NAME}")
        userver_testsuite_add_simple(
            CONFIG_PATH "${CONFIG_PATH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        message(WARNING "Target ${PROJECT_NAME} not found. Skipping testsuite setup.")
    endif()
endfunction()
