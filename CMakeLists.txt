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

# Define directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Option to enable eBPF metrics (default: OFF)
option(ENABLE_EBPF_METRICS "Enable eBPF metrics collection" OFF)

# Check for eBPF dependencies if enabled
if(ENABLE_EBPF_METRICS)
    message(STATUS "eBPF metrics collection: ENABLED")
    
    # Check for minimal BCC headers
    set(BCC_MINIMAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/bcc_minimal")
    if(EXISTS "${BCC_MINIMAL_DIR}/include/bcc/bcc_common.h")
        message(STATUS "Using minimal BCC headers from ${BCC_MINIMAL_DIR}")
        set(BCC_INCLUDE_DIRS "${BCC_MINIMAL_DIR}/include")
        set(BCC_FOUND TRUE)
        add_definitions(-DHAVE_BCC -DENABLE_EBPF_METRICS)
    endif()
    
    # Check for BCC submodule
    set(BCC_SUBMODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/bcc")
    if(EXISTS "${BCC_SUBMODULE_DIR}/src/cc/libbcc.h" AND NOT BCC_FOUND)
        message(STATUS "Found BCC submodule, will use it for include paths")
        set(BCC_INCLUDE_DIRS "${BCC_SUBMODULE_DIR}/src")
        add_subdirectory(${BCC_SUBMODULE_DIR} EXCLUDE_FROM_ALL)
        set(HAVE_BCC_SUBMODULE TRUE)
        add_definitions(-DHAVE_BCC -DENABLE_EBPF_METRICS)
    endif()
    
    # Check if we found BCC
    if(NOT BCC_FOUND AND NOT HAVE_BCC_SUBMODULE)
        message(WARNING "BCC not found, disabling eBPF metrics")
        set(ENABLE_EBPF_METRICS OFF)
    endif()
else()
    message(STATUS "eBPF metrics collection: DISABLED")
    message(STATUS "To enable, configure with -DENABLE_EBPF_METRICS=ON")
endif()

# Set output directories for all builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Find all source files
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# Create executable target
add_executable(tcp_comparison_linux ${SRC_FILES})

# Set include directories
target_include_directories(tcp_comparison_linux PRIVATE ${INCLUDE_DIR})

# Add BCC support to the target if enabled
if(ENABLE_EBPF_METRICS)
    if(BCC_FOUND)
        target_include_directories(tcp_comparison_linux PRIVATE ${BCC_INCLUDE_DIRS})
        if(BCC_LIBRARIES)
            target_link_libraries(tcp_comparison_linux PRIVATE ${BCC_LIBRARIES})
        endif()
        message(STATUS "Using BCC from system or minimal headers")
    elseif(HAVE_BCC_SUBMODULE)
        target_include_directories(tcp_comparison_linux PRIVATE ${BCC_INCLUDE_DIRS})
        target_link_libraries(tcp_comparison_linux PRIVATE bcc-shared)
        message(STATUS "Using BCC from submodule")
    endif()
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

# Output information for debugging
message(STATUS "Source files: ${SRC_FILES}")