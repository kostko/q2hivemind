
add_subdirectory(network)
add_subdirectory(mapping)

set(hivemind_src
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

  # External libraries
  ${Boost_LIBRARIES}
  rt
)

