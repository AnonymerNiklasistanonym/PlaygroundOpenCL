#! /usr/bin/env bash

# Check if cmake is installed
if ! [ -x "$(command -v cmake)" ]; then
  echo 'Error: cmake is not installed.' >&2
  exit 1
else
  echo -n "CMake version target: 3.5 => found: "
  cmake --version | head -n 1
fi

# Check if make is installed
if ! [ -x "$(command -v make)" ]; then
  echo 'Error: make is not installed.' >&2
  exit 1
else
  echo -n "Make version target: 4.1 => found: "
  make --version | head -n 1
fi

# Create OpenCL C++ headers if they are not already existing
./createOpenClHeaders.sh

# Create directory for out of source build
OUT_OF_SOURCE_BUILD_DIRECTORY=build/
if [[ -d "$OUT_OF_SOURCE_BUILD_DIRECTORY" ]]; then
    echo "The CMAKE build directory was already found ($OUT_OF_SOURCE_BUILD_DIRECTORY), this will be a faster build"
    echo "[Remove the $OUT_OF_SOURCE_BUILD_DIRECTORY directory to build them again from scratch]"
else
    mkdir $OUT_OF_SOURCE_BUILD_DIRECTORY
fi
cd $OUT_OF_SOURCE_BUILD_DIRECTORY

# Run CMake to create Makefile
CMAKE_BUILD_TYPE=Release
if [[ "$OSTYPE" == "linux-gnu" ]] || [[ "$OSTYPE" == "linux-ms" ]]; then
	echo "CMake: Linux -> Unix Makefiles"
	cmake .. -G "Unix Makefiles" -Wdev -Wdeprecated -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
elif [[ "$OSTYPE" == "msys" ]]; then
	echo "CMake: Windows MinGW -> MinGW Makefiles"
	cmake .. -G "MinGW Makefiles" -Wdev -Wdeprecated -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
else
	echo "Platform not supported! ($OSTYPE)" >&2
	exit 1
fi

# Build executable
make
if ! [[ $? -eq 0 ]]; then
	echo "Make was not successful!" >&2
	exit 1
fi

# Copy executable to dist directory
mkdir -p ../dist
cp main ../dist/main

# Run it with
# cd ..
# ./dist/main
