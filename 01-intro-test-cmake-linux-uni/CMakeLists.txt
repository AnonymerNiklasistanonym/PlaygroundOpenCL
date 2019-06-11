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
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Make edge case for MSVC compiler (no Werror support)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /pedantic")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Werror")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_WARN_ARGS}")

# Get project headers and sources
set(PROJECT_SOURCE_DIR src/)
set(PROJECT_HEADER_DIR src/)
set(PROJECT_INCLUDE include/)
file(GLOB PROJECT_SOURCES "${PROJECT_SOURCE_DIR}*.cpp")
set(PROJECT_MAIN ${PROJECT_SOURCE_DIR}main.cpp)

# Create executable with the following source files:
add_executable(${PROJECT_NAME} ${PROJECT_MAIN} ${PROJECT_SOURCES} ${OpenCL_INCLUDE_DIR} ${PROJECT_INCLUDE})

# Include the project headers
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_HEADER_DIR})

# Include OpenCL headers, the locally created OpenCL headers (this will mean they will not be checked for errors)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${OpenCL_INCLUDE_DIR} ${PROJECT_INCLUDE})

# Link the project with the OpenCL libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCL_LIBRARY})

# Copy the kernel file
configure_file(src/kernels/kernel.cl ${CMAKE_BINARY_DIR}/kernels/kernel.cl COPYONLY)
configure_file(src/kernels/kernel_helper.cl ${CMAKE_BINARY_DIR}/kernels/kernel_helper.cl COPYONLY)
