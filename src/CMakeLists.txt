
add_subdirectory(mold)
add_subdirectory(network)
add_subdirectory(mapping)
add_subdirectory(planner)
add_subdirectory(states)
add_subdirectory(rl)
add_subdirectory(brains)
add_subdirectory(eann)
add_subdirectory(fuzzy)
add_subdirectory(voters)

set(hivemind_core_src
algebra.cpp
logger.cpp
object.cpp
context.cpp
event.cpp
dispatcher.cpp
)

set(hivemind_libraries
  # Internal libraries
  mold
  network
  mapping
  planner
  states
  rl
  brains
  eann
  fuzzy
  voters

  # External libraries
  ${PROTOBUF_LIBRARY}
  ${Boost_LIBRARIES}
  rt
)

add_library(hivemind_core STATIC ${hivemind_core_src})
add_dependencies(hivemind_core mold)

add_subdirectory(simulation)

add_executable(hivemind main.cpp)
target_link_libraries(hivemind hivemind_core ${hivemind_libraries}
hivemind_core mold)

