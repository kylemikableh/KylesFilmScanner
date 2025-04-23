# Scanner

## Build for MAC OS
Install vcpkg and install the packages
Make build folder
From inside build folder, run `cmake .. -DCMAKE_TOOLCHAIN_FILE=/Users/kyle/c++/vcpkg/scripts/buildsystems/vcpkg.cmake -G Xcode` except replace your toolchain link with the one from vcpkg integrate, and generating an xcode project in the build folder to use for profiling 
then `make` if doing using terminal, or build in xcode

## Build for Ubuntu/Linux

## Build for Windows