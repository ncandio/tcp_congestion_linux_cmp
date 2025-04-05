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

# Setup module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Set up eBPF support
include(SetupBPF)

# Define directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Check for local bcc installation as git submodule
set(BCC_SUBMODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/bcc")
if(EXISTS "${BCC_SUBMODULE_DIR}/src/cc/libbcc.h" AND NOT BCC_FOUND)
    message(STATUS "Found BCC submodule, will use it for include paths")
    set(BCC_INCLUDE_DIRS "${BCC_SUBMODULE_DIR}/src")
    add_subdirectory(${BCC_SUBMODULE_DIR} EXCLUDE_FROM_ALL)
    set(HAVE_BCC_SUBMODULE TRUE)
endif()

# Set output directories for all builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# Find all source files
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)

# Create executable target
add_executable(tcp_comparison_linux ${SRC_FILES})

# Set include directories
target_include_directories(tcp_comparison_linux PRIVATE ${INCLUDE_DIR})

# Add BCC support to the target
if(EBPF_SUPPORT_AVAILABLE)
    add_ebpf_target(tcp_comparison_linux)
elseif(HAVE_BCC_SUBMODULE)
    target_include_directories(tcp_comparison_linux PRIVATE ${BCC_INCLUDE_DIRS})
    target_link_libraries(tcp_comparison_linux PRIVATE bcc-shared)
    add_definitions(-DHAVE_BCC -DENABLE_EBPF_METRICS)
    message(STATUS "Using BCC from submodule")
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
