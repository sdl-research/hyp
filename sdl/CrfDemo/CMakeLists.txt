project (CrfDemo)

sdl_util_includes()
include_directories(${LBFGS_INCLUDE_DIR})

set(LINK_DEPENDENCIES
  ${LBFGS_LIB}
  ${YAML_CPP_LIBRARY}
  Config
  Hypergraph
  ${UTIL_LIBRARIES}
  Optimization
  )

sdl_add_library( ${PROJECT_NAME}-shared ${PROJECT_SOURCE_DIR} SHARED)
target_link_libraries(${PROJECT_NAME}-shared ${LINK_DEPENDENCIES})
