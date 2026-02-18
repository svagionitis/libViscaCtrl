cmake_minimum_required(VERSION 3.16)

project(ViscaCtrl VERSION 1.0.0 LANGUAGES CXX)

# A CPP compiler is absolutely needed
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Specify whether compiler specific extensions are requested
set(CMAKE_CXX_EXTENSIONS OFF)
# Enable the compile_commands.json
# See https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake/Modules"
    "${CMAKE_SOURCE_DIR}/cmake")

# For windows, this needs to be changed to OFF I believe
option(NON_TRANSITIVE "Option to use non-transitive linking" OFF)
option(ENABLE_CL_CLI "Build command line client" ON)
option(ENABLE_QT_CLI "Build Qt UI client" OFF)
# Option to build shared or static library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
# For Windows, we need to set the appropriate defines
if(BUILD_SHARED_LIBS AND WIN32)
    add_compile_definitions(VISCA_BUILD_SHARED)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    find_package(Git)
endif()

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their
# definition and dump it in the cache along with proper documentation,
# otherwise set CMAKE_BUILD_TYPE to Release prior to calling PROJECT()
#
# See https://cmake.org/pipermail/cmake/2008-September/023808.html
#     https://cmake.org/pipermail/cmake/2009-June/030311.html
# License: BSD 3-clause
# Author: Philip Lowman
#
if (CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE
       STRING "Choose the type of build, options are:
- None (CMAKE_CXX_FLAGS)
- Debug
- Release
- RelWithDebInfo
- MinSizeRel.")
else ()
   set(CMAKE_BUILD_TYPE Release CACHE
       STRING "Choose the type of build, options are:
- None (CMAKE_CXX_FLAGS)
- Debug
- Release
- RelWithDebInfo
- MinSizeRel."
       FORCE)
endif ()

# Find the clang-format executable
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_FORMAT_EXE)
    # Get all project files (adjust extensions as needed)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.hpp *.cc)

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting code with WebKit style..."
    )
else()
    message(WARNING "clang-format not found. The 'format' target will not be available.")
endif()

add_subdirectory(lib)

if (ENABLE_CL_CLI)
    add_subdirectory(ClViscaCli)
endif ()

if (ENABLE_QT_CLI)
    add_subdirectory(QtViscaCli)
endif ()

# Add GoogleTest support
include(cmake/AddGTest.cmake)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (MSVC)
    message (STATUS "If using the 'Visual Studio' generator, now run")
    message (STATUS "     cmake --build . [target] -- /maxcpucount[:N]")
    message (STATUS "If using the 'NMake Makefiles' generator, now run")
    message (STATUS "     cmake --build . [target]")
else ()
    message (STATUS "If using the 'Unix Makefiles' generator, now run")
    message (STATUS "     cmake --build . [target] -- -j$(nproc)")
endif ()
