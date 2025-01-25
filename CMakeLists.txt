add_executable( Scanner Scanner.cpp)
target_link_libraries( Scanner PRIVATE pylon::pylon )
install( TARGETS Scanner )

find_package(OpenImageIO CONFIG REQUIRED)
target_link_libraries(main PRIVATE OpenImageIO::OpenImageIO)