add_library(topologies INTERFACE)

target_include_directories(topologies
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(topologies
  INTERFACE
    helpers
)

target_compile_features(topologies
  INTERFACE
    cxx_std_23
)
