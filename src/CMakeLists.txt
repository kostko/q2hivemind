
#add_subdirectory(network)

set(hivemind_src
logger.cpp
object.cpp
context.cpp
main.cpp
)

add_executable(hivemind ${hivemind_src})

# External libraries
target_link_libraries(hivemind
  ${Boost_LIBRARIES}
)

