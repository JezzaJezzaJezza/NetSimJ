add_library(engines INTERFACE)

target_include_directories(engines
  INTERFACE
    ${PROJECT_SOURCE_DIR}/modules
)

target_link_libraries(engines
  INTERFACE
    helpers
    topologies
    traffic
)

target_compile_features(engines
  INTERFACE
    cxx_std_23
)
