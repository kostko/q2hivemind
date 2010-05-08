
add_subdirectory(network)
add_subdirectory(mapping)
add_subdirectory(planner)
add_subdirectory(states)
add_subdirectory(rl)

set(hivemind_src
algebra.cpp
logger.cpp
object.cpp
context.cpp
event.cpp
dispatcher.cpp
main.cpp
)

add_executable(hivemind ${hivemind_src})

target_link_libraries(hivemind
  # Internal libraries
  network
  mapping
  planner
  states
  rl

  # External libraries
  ${Boost_LIBRARIES}
  rt
)

