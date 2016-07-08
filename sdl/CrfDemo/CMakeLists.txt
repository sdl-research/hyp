project (CrfDemo)

sdl_util_includes()
include_directories(${LBFGS_INCLUDE_DIR})

set(LINK_DEPENDENCIES
  Hypergraph Vocabulary Optimization
  ${CONFIG_LIBRARIES}
  ${YAML_CPP_LIBRARY} ${LBFGS_LIB}
  )

sdl_add_lib(${PROJECT_NAME}-shared SHARED src/CreateSearchSpace.cpp)
sdl_target_libs(${PROJECT_NAME}-shared ${LINK_DEPENDENCIES})
