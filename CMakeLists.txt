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
        include/BinaryIO.h
        include/CommandType.h
        include/CommandMap.h
        include/CommandHandlers.h)

# --- Add benchmark executable ---
file(GLOB SRC_FILES "src/*.cpp")
list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp")
add_executable(benchmark_table_index tests/benchmark_table_index.cpp ${SRC_FILES})
target_include_directories(benchmark_table_index PRIVATE ${PROJECT_SOURCE_DIR}/include)
