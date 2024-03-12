@echo off

rmdir /S build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=C:/Users/Space/Documents/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release