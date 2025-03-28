add_subdirectory(userver)

userver_setup_environment()

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

if(BUILD_TESTS)
    userver_testsuite_add_simple()
endif()
