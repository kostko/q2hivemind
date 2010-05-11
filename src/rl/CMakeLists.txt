set(rl_src
enumvector.cpp
action.cpp
brainstate.cpp
statespace.cpp
brains.cpp
)

add_library(rl STATIC ${rl_src})
add_dependencies(rl planner states)
