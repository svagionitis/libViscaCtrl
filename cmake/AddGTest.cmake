# This file handles GoogleTest integration
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
    # Try to find system-installed GTest first
    find_package(GTest QUIET)
    find_package(GMock QUIET)

    if(GTEST_FOUND AND GMOCK_FOUND)
        message(STATUS "Using system GoogleTest")
        # Add the same function as above but using system libraries
        function(ADD_GTEST TEST_NAME TEST_SOURCES LINK_LIBRARIES INCLUDE_DIRECTORIES)
            add_executable(${TEST_NAME} ${TEST_SOURCES})
            target_link_libraries(${TEST_NAME}
                PRIVATE
                    ${LINK_LIBRARIES}
                    GTest::GTest
                    GTest::Main
                    GMock::GMock
            )
            target_include_directories(${TEST_NAME}
                PRIVATE
                    ${INCLUDE_DIRECTORIES}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${GTEST_INCLUDE_DIRS}
                    ${GMOCK_INCLUDE_DIRS}
            )
            include(GoogleTest)
            gtest_discover_tests(${TEST_NAME})
        endfunction()
    else()
        # Fall back to FetchContent
        include(FetchContent)

        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            # Or use Git:
            # GIT_REPOSITORY https://github.com/google/googletest.git
            # GIT_TAG v1.14.0
        )

        if(WIN32)
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        endif()

        FetchContent_MakeAvailable(googletest)

        function(ADD_GTEST TEST_NAME TEST_SOURCES LINK_LIBRARIES INCLUDE_DIRECTORIES)
            add_executable(${TEST_NAME} ${TEST_SOURCES})
            target_link_libraries(${TEST_NAME}
                PRIVATE
                    ${LINK_LIBRARIES}
                    GTest::gtest_main
                    GTest::gmock
            )
            target_include_directories(${TEST_NAME}
                PRIVATE
                    ${INCLUDE_DIRECTORIES}
                    ${CMAKE_CURRENT_SOURCE_DIR}
            )
            if(MSVC)
                target_compile_options(${TEST_NAME} PRIVATE /W4)
            endif()
            include(GoogleTest)
            gtest_discover_tests(${TEST_NAME})
        endfunction()
    endif()
endif()
