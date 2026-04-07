cmake_minimum_required(VERSION 3.20)

option(ENABLE_CUDA "Build with CUDA GPU engine support" OFF)

project(
  NetSimJ
  VERSION 1.0
  LANGUAGES CXX
)

if(ENABLE_CUDA)
  enable_language(CUDA)
  set(CMAKE_CUDA_STANDARD 17)
  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(modules)


add_executable(NetSim src/main.cpp)

target_link_libraries(NetSim
  PRIVATE
    topologies
    engines
    traffic
    routers
)

if(ENABLE_CUDA)
  target_link_libraries(NetSim PRIVATE cuda_engine)
  target_compile_definitions(NetSim PRIVATE NETSIM_HAS_CUDA=1)
endif()


