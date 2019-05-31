#! /usr/bin/env bash

# Check if cmake is installed
if ! [ -x "$(command -v cmake)" ]; then
  echo 'Error: cmake is not installed.' >&2
  exit 1
else
  echo -n "CMake version target: 3.14.2 => found: "
  cmake --version | head -n 1
fi

# Check if make is installed
if ! [ -x "$(command -v make)" ]; then
  echo 'Error: make is not installed.' >&2
  exit 1
else
  echo -n "Make version target: 4.2.1 => found: "
  make --version | head -n 1
fi

# OpenCL paths (Windows, Intel SDK)
OPENCL_SDK_PATH="/c/Program Files (x86)/IntelSWTools/OpenCL/sdk"
# Directory that contains the directory CL and in it the headers
OPENCL_INCLUDE_PATH="${OPENCL_SDK_PATH}/include"
# Directory which contains the /x64/OpenCL.lib file
OPENCL_LIBRARY_PATH="${OPENCL_SDK_PATH}/lib"

# Copy OpenCL
rm -rf include
rm -rf lib
cp -R "$OPENCL_INCLUDE_PATH" ./include
cp -R "$OPENCL_LIBRARY_PATH" ./lib

# Create directory for out of source build
rm -rf build
mkdir -p build
cd build

# Run CMake to create Makefile
echo "CMake: Windows MinGW -> MinGW Makefiles"
cmake .. -G "MinGW Makefiles" -Wdev -Wdeprecated -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_BUILD_TYPE=Release

# Build executable
make

# Copy executable to dist directory
mkdir -p ../dist
cp main ../dist/main

# Run it with
# ./dist/main
