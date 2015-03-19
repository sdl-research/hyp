project (CrfDemo)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${LOG4CXX_INCLUDE_DIR})
include_directories(${LBFGS_INCLUDE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR})
include_directories(${TBB_INCLUDE_DIR})

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

