cmake_minimum_required(VERSION 3.16)

# Automatically add the current source and build directories to the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Define sources
set(VISCA_SOURCES
    ${CMAKE_SOURCE_DIR}/lib/Commands.h
    ${CMAKE_SOURCE_DIR}/lib/Commands.cpp
    ${CMAKE_SOURCE_DIR}/lib/Export.h
    ${CMAKE_SOURCE_DIR}/lib/ICommunicator.h
    ${CMAKE_SOURCE_DIR}/lib/Logger.h
    ${CMAKE_SOURCE_DIR}/lib/Logger.cpp
    ${CMAKE_SOURCE_DIR}/lib/RingBuffer.h
    ${CMAKE_SOURCE_DIR}/lib/ViscaController.h
    ${CMAKE_SOURCE_DIR}/lib/ViscaController.cpp
    ${CMAKE_SOURCE_DIR}/lib/SerialCommunicator.h
    ${CMAKE_SOURCE_DIR}/lib/TcpCommunicator.h
    ${CMAKE_SOURCE_DIR}/lib/UdpCommunicator.h
    ${CMAKE_SOURCE_DIR}/lib/UtilsCommon.h
)

if(WIN32)
    list(APPEND VISCA_SOURCES
        ${CMAKE_SOURCE_DIR}/lib/SerialCommunicator_windows.cpp
        ${CMAKE_SOURCE_DIR}/lib/TcpCommunicator_windows.cpp
        ${CMAKE_SOURCE_DIR}/lib/UdpCommunicator_windows.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND VISCA_SOURCES
        ${CMAKE_SOURCE_DIR}/lib/SerialCommunicator_linux.cpp
        ${CMAKE_SOURCE_DIR}/lib/TcpCommunicator_linux.cpp
        ${CMAKE_SOURCE_DIR}/lib/UdpCommunicator_linux.cpp
    )
endif()

set(LINK_LIBRARIES
    Threads::Threads)

# Detect OS and set platform-specific flags/files
if(WIN32)
    # Windows specific configurations if needed
    add_definitions(-D_WIN32_WINNT=0x0601) # Target Windows 7+
    list(APPEND LINK_LIBRARIES
         ws2_32
         setupapi)
elseif(UNIX AND NOT APPLE)
    # Linux specific configurations
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(Threads REQUIRED)

# Create the library based on options
if(BUILD_SHARED_LIBS)
    add_library(${CMAKE_PROJECT_NAME} SHARED ${VISCA_SOURCES})

    # For shared library on Unix, set visibility
    if(UNIX AND NOT APPLE)
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fvisibility=hidden)
    endif()
else()
    add_library(${CMAKE_PROJECT_NAME} STATIC ${VISCA_SOURCES})
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

# Ensure headers in lib are visible
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Set output directory
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
