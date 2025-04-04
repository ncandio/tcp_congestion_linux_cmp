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

# Initialize eBPF support flag
set(ENABLE_EBPF_METRICS OFF)

# Check for all required eBPF dependencies
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(BCC QUIET libbcc)
    
    # Check for Python3
    execute_process(
        COMMAND bash -c "command -v python3"
        OUTPUT_VARIABLE PYTHON3_PATH
        RESULT_VARIABLE PYTHON3_RESULT
        ERROR_QUIET
    )
    
    # Check for pandas
    execute_process(
        COMMAND bash -c "python3 -c 'import pandas'"
        RESULT_VARIABLE PANDAS_RESULT
        ERROR_QUIET
    )
    
    # Enable eBPF if all dependencies are present
    if(BCC_FOUND AND PYTHON3_RESULT EQUAL 0 AND PANDAS_RESULT EQUAL 0)
        set(ENABLE_EBPF_METRICS ON)
        add_definitions(-DHAVE_BCC)
        add_definitions(-DENABLE_EBPF_METRICS)
        message(STATUS "eBPF support fully enabled - all dependencies found")
    else()
        message(STATUS "eBPF support disabled - missing dependencies:")
        if(NOT BCC_FOUND)
            message(STATUS "  - BCC not found. Install with: sudo apt-get install bpfcc-tools python3-bpfcc")
        endif()
        if(NOT PYTHON3_RESULT EQUAL 0)
            message(STATUS "  - Python3 not found")
        endif()
        if(NOT PANDAS_RESULT EQUAL 0)
            message(STATUS "  - pandas Python module not found. Install with: pip3 install pandas")
        endif()
    endif()
else()
    message(STATUS "PkgConfig not found - eBPF support will be disabled")
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

# Summary of eBPF support status
if(ENABLE_EBPF_METRICS)
    message(STATUS "eBPF metrics collection: ENABLED")
else()
    message(STATUS "eBPF metrics collection: DISABLED")
    message(STATUS "Install all dependencies with: sudo apt-get install bpfcc-tools python3-bpfcc && pip3 install pandas")
endif()

# Output information for debugging
message(STATUS "Source files: ${SRC_FILES}")
