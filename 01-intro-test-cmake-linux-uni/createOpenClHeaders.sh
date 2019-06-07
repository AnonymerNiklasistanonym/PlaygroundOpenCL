#! /usr/bin/env bash

# Check if the headers were already created
MAIN_OPENCL_HEADER_FILE="include/CL/cl.hpp"
if [[ -f "$MAIN_OPENCL_HEADER_FILE" ]]; then
	echo "The main header file was already found ($MAIN_OPENCL_HEADER_FILE), do not build the headers again"
    echo "[Remove the include/CL directory to build them again]"
else
    SUBMODULE_OPENCL_HPP_DIR=vendor/OpenCL-CLHPP/
	echo "> CREATE OpenCL headers..."
    git submodule update --init
    cd $SUBMODULE_OPENCL_HPP_DIR
    git submodule update --init
    OPENCL_HPP_BUILD_DIR=build/
    if [[ -d "$OPENCL_HPP_BUILD_DIR" ]]; then
        echo "The CMAKE build directory was already found ($OPENCL_HPP_BUILD_DIR), this will be a faster build"
        echo "[Remove the $OPENCL_HPP_BUILD_DIR directory to build them again from scratch]"
    else
        mkdir $OPENCL_HPP_BUILD_DIR
    fi
    cd $OPENCL_HPP_BUILD_DIR
    cmake ..
    # Generate only cl.hpp
    make generate_clhpp
    echo "...OpenCL headers are created"
    cd ../../../
    echo "> COPY OpenCL headers..."
    mkdir -p include
    cp -R $SUBMODULE_OPENCL_HPP_DIR/$OPENCL_HPP_BUILD_DIR/include/CL include/CL
    echo "...OpenCL headers copied"
fi


