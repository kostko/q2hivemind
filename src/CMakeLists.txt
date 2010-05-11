
add_subdirectory(mold)
add_subdirectory(network)
add_subdirectory(mapping)
add_subdirectory(planner)
add_subdirectory(states)
add_subdirectory(rl)
add_subdirectory(brains)

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
  mold
  network
  mapping
  planner
  states
  rl
  brains

  # External libraries
  ${PROTOBUF_LIBRARY}
  ${Boost_LIBRARIES}
  rt
)

