cmake_minimum_required(VERSION 3.0)

project(hwio-server)


# C++14 standard should be safely usable
set(CMAKE_CXX_STANDARD 14)

#Warning flags for all builds
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# collect sources
set(hwio-server_SOURCES
   ./src/hwio_server_main.cpp
)

# find required libraries
find_package(hwio REQUIRED)

# define executable and dependencies
add_executable(hwio-server ${hwio-server_SOURCES})
target_link_libraries(hwio-server hwio)
target_link_libraries(hwio-server dl)

# define installation
install(TARGETS hwio-server DESTINATION bin)
