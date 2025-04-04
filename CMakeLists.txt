# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(tcp_congestion_linux_cmp VERSION 1.0 LANGUAGES CXX)

# Set C++17 standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warning flags for GCC/Clang
add_compile_options(-Wall -Wextra -pedantic)

# Set optimization level (Release builds will have -O2 or equivalent)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Find BCC (if available)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(BCC QUIET libbcc)
    if(BCC_FOUND)
        add_definitions(-DHAVE_BCC)
        message(STATUS "BCC found - eBPF support enabled")
    else()
        message(STATUS "BCC not found - eBPF support will be limited")
    endif()
endif()

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
if(BCC_FOUND)
    target_include_directories(tcp_comparison_linux PRIVATE ${BCC_INCLUDE_DIRS})
    target_link_libraries(tcp_comparison_linux PRIVATE ${BCC_LIBRARIES})
endif()

# Install rules
install(TARGETS tcp_comparison_linux 
        RUNTIME DESTINATION bin)

# Install eBPF scripts
install(FILES 
    ${SRC_DIR}/tcp_ebpf_metrics.c
    ${SRC_DIR}/tcp_ebpf_collector.py
    DESTINATION share/${PROJECT_NAME}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Install documentation
install(FILES
    ${SRC_DIR}/README_EBPF.md
    DESTINATION share/doc/${PROJECT_NAME}
)

# Check for eBPF dependencies during build
add_custom_target(check_ebpf_deps ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Checking for eBPF dependencies..."
    COMMAND bash -c "command -v python3 >/dev/null 2>&1 || echo 'WARNING: python3 not found, eBPF metrics collection will not work'"
    COMMAND bash -c "python3 -c 'import pandas 2>/dev/null' || echo 'WARNING: pandas not found, install with: pip3 install pandas'"
    COMMAND bash -c "[ -d /usr/include/bcc ] || echo 'WARNING: BCC headers not found, eBPF metrics collection will not work. Install with: sudo apt-get install bpfcc-tools python3-bpfcc'"
)

# Output information for debugging
message(STATUS "Source files: ${SRC_FILES}")
