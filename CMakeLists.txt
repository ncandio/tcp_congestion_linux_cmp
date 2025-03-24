# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(MyProject VERSION 1.0 LANGUAGES CXX)

# Set C++17 standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags for GCC/Clang
add_compile_options(-Wall -Wextra -pedantic)

# Set optimization level (Release builds will have -O2 or equivalent)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Define directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Set output directories for all builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Find all source files
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# Create executable target
add_executable(tcp_comparison_linux ${SRC_FILES})

# Set include directories
target_include_directories(tcp_comparison_linux PRIVATE ${INCLUDE_DIR})

# Optional: Add specific libraries if needed
# find_package(SomeLibrary REQUIRED)
# target_link_libraries(tcp_comparison_linux PRIVATE SomeLibrary::SomeLibrary)

# Install rules (optional)
install(TARGETS tcp_comparison_linux 
        RUNTIME DESTINATION bin)

# Output information for debugging (optional)
message(STATUS "Source files: ${SRC_FILES}")
