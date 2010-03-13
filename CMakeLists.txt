project(hivemind)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

SET(Boost_USE_MULTITHREAD ON)
SET(Boost_USE_STATIC_LIBS OFF)

find_package(Boost 1.37.0 COMPONENTS filesystem signals thread REQUIRED)
find_package(Eigen2 REQUIRED)

# Uncomment these lines for optimization
#SET(CMAKE_BUILD_TYPE distribution)
#SET(CMAKE_CXX_FLAGS_DISTRIBUTION "-O3 -msse2 -DNDEBUG")
#SET(CMAKE_C_FLAGS_DISTRIBUTION "-O3 -msse2 -DNDEBUG")

# Uncomment these lines for full debugging
SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_CXX_FLAGS_DEBUG "-g3")
SET(CMAKE_C_FLAGS_DEBUG "-g3")

include_directories(
        # External includes
        ${Boost_INCLUDE_DIRS}
        ${EIGEN2_INCLUDE_DIR}
        
        # Internal includes
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(src)

