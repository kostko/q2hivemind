
add_subdirectory(network)
add_subdirectory(mapping)
add_subdirectory(planner)
add_subdirectory(states)

set(hivemind_src
algebra.cpp
logger.cpp
object.cpp
context.cpp
main.cpp
)

add_executable(hivemind ${hivemind_src})

target_link_libraries(hivemind
  # Internal libraries
  network
  mapping
  planner
  states

  # External libraries
  ${Boost_LIBRARIES}
  rt
)

