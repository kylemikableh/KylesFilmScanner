add_executable( Scanner Scanner.cpp)
target_link_libraries( Scanner PRIVATE pylon::pylon )
install( TARGETS Scanner )