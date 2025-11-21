add_library(engines INTERFACE)

target_include_directories(engines
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(engines
  INTERFACE
    cxx_std_23
)
