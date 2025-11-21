add_library(traffic INTERFACE)

target_include_directories(traffic
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(traffic
  INTERFACE
    cxx_std_23
)
