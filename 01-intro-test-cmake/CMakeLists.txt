# Minimum CMake version
cmake_minimum_required (VERSION 3.14)

# Set name of project
project(main LANGUAGES CXX)

# Set C++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include OpenCL headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Create the executable with the following source files:
add_executable(main main.cpp)

# Include the 64bit OpenCL.lib file
target_link_libraries(main ${CMAKE_SOURCE_DIR}/lib/x64/OpenCL.lib)
