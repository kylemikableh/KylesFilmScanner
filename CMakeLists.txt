cmake_minimum_required(VERSION 3.15)
project(Scanner)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(/Library/Frameworks/pylon.framework/Headers/ /Library/Frameworks/pylon.framework/Headers/GenICam/)

set(BESLER_LIBRARIES
 pylonbase
 pylonutility
 GenApi_gcc_v3_1_Basler_pylon_v5_1
 GCBase_gcc_v3_1_Basler_pylon_v5_1
)

link_directories(/Library/Frameworks/pylon.framework/Libraries/)

# Use vcpkg toolchain if available
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Set Pylon SDK path if not in a standard location
set(Pylon_DIR "/Library/Frameworks/pylon.framework/Versions/A/Resources/CMake")

# Add executable and source files
add_executable(Scanner
    Scanner.cpp
    SerialConn.cpp
    ImageCaptureController.cpp
    ImagesProcessor.cpp
    RGBImage.cpp
    RGBImageQueue.cpp
    MDriveConn.cpp

    # Add headers for easy access
    SerialConn.h
    ImageCaptureController.h
    ImagesProcessor.h
    RGBImage.h
    RGBImageQueue.h
    MDriveConn.h
    
    # Add any other source files here
)

# Include directories
target_include_directories(Scanner PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${BESLER_LIBRARIES}
)

# Link libraries
find_package(OpenImageIO CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(Pylon REQUIRED)

target_link_libraries(Scanner PRIVATE
    OpenImageIO::OpenImageIO
    Boost::system
    Boost::thread
    pylon::pylon
)

# Set additional compiler options
# target_compile_definitions(Scanner PRIVATE
#     WIN32
#     _CONSOLE
#     _WIN32_WINNT=0x0A00
# )

# Set output directories
# set_target_properties(Scanner PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#     ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
# )

# Install target
# install(TARGETS Scanner DESTINATION bin)