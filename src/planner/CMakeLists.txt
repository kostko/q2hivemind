
set(planner_src
local.cpp
state.cpp
global.cpp
)

add_library(planner STATIC ${planner_src})
add_dependencies(planner mold rl)
target_link_libraries(planner mold rl)

