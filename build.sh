#!/bin/bash

# Set script to fail on error
set -e

# Set build directory
BUILD_DIR="build"

# Create build directory if it doesn't exist
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure the project with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TEST=ON

# Build the project using all CPU cores
cmake --build . --parallel

# Optionally run tests
ctest
