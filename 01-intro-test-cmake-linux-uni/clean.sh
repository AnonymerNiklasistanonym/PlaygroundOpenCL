#!/usr/bin/env bash

# Remove build/dist directories
find ./ -type d -regex ".*\(build.*\|dist\|out\)" \
     -print0 | xargs -n 1 -0 rm -rf

# Remove Visual Studio / Code directories
find ./ -type d -regex ".*\.\(vs\|vscode\)" \
     -print0 | xargs -n 1 -0 rm -rf

# Remove weird .orig files and .user files
find ./ -type f -regex ".*\.\(orig\|user\)" \
     -print0 | xargs -n 1 -0 rm -f

# Remove OpenCL include directory
rm -rf include/CL

# Reset repository
cd vendor/openclcppheader/
git fetch origin
git reset --hard origin/master
cd ..
