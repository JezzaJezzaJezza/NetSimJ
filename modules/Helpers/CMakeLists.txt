add_library(helpers INTERFACE)

target_include_directories(helpers
  INTERFACE
    ${PROJECT_SOURCE_DIR}/modules
)

target_compile_features(helpers
  INTERFACE
    cxx_std_23
)
  
