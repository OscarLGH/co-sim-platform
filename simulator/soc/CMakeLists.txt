cmake_minimum_required(VERSION 3.10)
project(soc_simulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    soc_top.cc
    ip.cc
    ram.cc
    cosim_bridge.cc
)

# Header files
set(HEADERS
    bus.hh
    cosim_bridge.hh
    debugger.hh
    ip.hh
    ram.hh
    soc_top.hh
)

# Create executable
add_executable(soc.out ${SOURCES} ${HEADERS})

# Link pthread
target_link_libraries(soc.out pthread)

# Optional: Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soc.out PRIVATE -Wall -Wextra)
endif()
