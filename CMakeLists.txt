cmake_minimum_required(VERSION 3.0)

project(hwio_server)


# C++14 standard should be safely usable
set(CMAKE_CXX_STANDARD 14)

#Warning flags for all builds
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# collect sources
set(HWIO_SERVER_SOURCES
   ./src/hwio_server_main.cpp
)

# find required libraries
find_package(hwio REQUIRED)

# define executable and dependencies
add_executable(hwio_server ${HWIO_SERVER_SOURCES})
target_link_libraries(hwio_server hwio)

# define installation
install(TARGETS hwio_server DESTINATION bin)
