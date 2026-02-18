cmake_minimum_required(VERSION 3.16)

project(ClViscaCli VERSION 1.0.0 LANGUAGES CXX)

# A CPP compiler is absolutely needed
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Specify whether compiler specific extensions are requested
set(CMAKE_CXX_EXTENSIONS OFF)
# Enable the compile_commands.json
# See https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Automatically add the current source and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Define sources
set(CL_VISCA_CLI_SOURCES
    ${CMAKE_SOURCE_DIR}/ClViscaCli/main.cpp
)

# Detect OS and set platform-specific flags/files
if(WIN32)
    # Windows specific configurations if needed
    add_definitions(-D_WIN32_WINNT=0x0601) # Target Windows 7+
elseif(UNIX AND NOT APPLE)
    # Linux specific configurations
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

add_executable(${PROJECT_NAME} ${CL_VISCA_CLI_SOURCES})
add_dependencies(${PROJECT_NAME} ${CMAKE_PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)
