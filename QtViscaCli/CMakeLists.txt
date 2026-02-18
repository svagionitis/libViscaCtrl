cmake_minimum_required(VERSION 3.16)

project(QtViscaCli VERSION 1.0.0 LANGUAGES CXX)

# A CPP compiler is absolutely needed
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Specify whether compiler specific extensions are requested
set(CMAKE_CXX_EXTENSIONS OFF)
# Enable the compile_commands.json
# See https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
