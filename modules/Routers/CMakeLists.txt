add_library(routers INTERFACE)

target_include_directories(routers
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(routers
  INTERFACE
    cxx_std_23
)
