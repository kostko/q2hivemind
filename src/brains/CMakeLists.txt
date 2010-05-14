set(brains_src
soldier.cpp
)

add_library(brains STATIC ${brains_src})
add_dependencies(brains rl)
target_link_libraries(brains rl states)

