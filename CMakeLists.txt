cmake_minimum_required(VERSION 3.16)
project(MarinaDB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory for headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Gather all .cpp files in src/
file(GLOB SOURCES "src/*.cpp")

add_executable(MarinaDB ${SOURCES}
        include/BinaryIO.h)
