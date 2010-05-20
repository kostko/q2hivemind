
set(planner_src
local.cpp
state.cpp
global.cpp
directory.cpp
sensors.cpp
motion.cpp
)

add_library(planner STATIC ${planner_src})
add_dependencies(planner mold)

