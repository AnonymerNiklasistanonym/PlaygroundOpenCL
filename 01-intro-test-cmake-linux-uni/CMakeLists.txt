# Minimum CMake version
cmake_minimum_required (VERSION 3.5)

# Set name of project
project(main LANGUAGES CXX)

# Set C++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCL libraries to include and link
find_package(OpenCL 1.2 REQUIRED)
message("-> OpenCL_INCLUDE_DIR: ${OpenCL_INCLUDE_DIR}")
message("-> OpenCL_LIBRARY:     ${OpenCL_LIBRARY}")

# Set compiler warn arguments
set(COMPILER_WARN_ARGS "-Wall -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_WARN_ARGS}")

# Get project headers and sources
set(PROJECT_SOURCE_DIR src/)
set(PROJECT_HEADER_DIR src/)
set(PROJECT_INCLUDE include/)
file(GLOB PROJECT_SOURCES "${PROJECT_SOURCE_DIR}*.cpp")
set(PROJECT_MAIN ${PROJECT_SOURCE_DIR}main.cpp)

# Create executable with the following source files:
add_executable(${PROJECT_NAME} ${PROJECT_MAIN} ${PROJECT_SOURCES})

# Include OpenCL headers, the locally created OpenCL headers and the headers of this project
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCL_INCLUDE_DIR} ${PROJECT_INCLUDE} ${PROJECT_HEADER_DIR})

# Link the project with the OpenCL libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCL_LIBRARY})
