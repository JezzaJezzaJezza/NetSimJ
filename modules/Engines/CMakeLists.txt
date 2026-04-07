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

if(ENABLE_CUDA)
  add_library(cuda_engine STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/CudaEngine.cu
  )
  target_include_directories(cuda_engine
    PUBLIC
      ${PROJECT_SOURCE_DIR}/modules
  )
  set_target_properties(cuda_engine PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
  )
  target_link_libraries(cuda_engine PUBLIC helpers)
endif()
