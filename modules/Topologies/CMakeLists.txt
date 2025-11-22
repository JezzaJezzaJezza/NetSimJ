add_library(topologies INTERFACE)

target_include_directories(topologies
  INTERFACE
    ${PROJECT_SOURCE_DIR}/modules
)

target_link_libraries(topologies
  INTERFACE
    helpers
)

target_compile_features(topologies
  INTERFACE
    cxx_std_23
)
