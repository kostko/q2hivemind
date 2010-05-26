
set(states_src
wander.cpp
swim.cpp
shoot.cpp
respawn.cpp
goto.cpp
gotoammo.cpp
gotohealth.cpp
gotoupgrade.cpp
gotoweapon.cpp
)

add_library(states STATIC ${states_src})

