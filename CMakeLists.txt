cmake_minimum_required(VERSION 3.0.0)

project(MemoryAccess VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(MemoryAccess
    src/main.cpp
    src/spi_device.cpp
)

target_include_directories(MemoryAccess PRIVATE includes)

set_target_properties(MemoryAccess PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin"
    ARCHIVE_OUTPUT_DIRECTORY "lib"
    LIBRARY_OUTPUT_DIRECTORY "lib"
)