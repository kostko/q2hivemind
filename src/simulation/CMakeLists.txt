
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/quake2)

set(simulation_src
interface.c
util.c
glue.cpp
)

add_library(simulation STATIC ${simulation_src})

add_subdirectory(quake2)

