cmake_minimum_required(VERSION 3.20)

project(
  NetSimJ
  VERSION 1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(modules)


add_executable(NetSim src/main.cpp)

target_link_libraries(NetSim 
  PRIVATE 
    topologies
)


