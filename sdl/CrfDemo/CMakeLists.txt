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

  Config
  Hypergraph

  Optimization
  )

add_library( ${PROJECT_NAME}-shared SHARED 
  ${PROJECT_SOURCE_DIR}/src/CreateSearchSpace.cpp)
target_link_libraries(${PROJECT_NAME}-shared ${LINK_DEPENDENCIES})

