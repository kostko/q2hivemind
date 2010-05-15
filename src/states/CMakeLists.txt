
set(states_src
wander.cpp
swim.cpp
shoot.cpp
respawn.cpp
)

add_library(states STATIC ${states_src})

