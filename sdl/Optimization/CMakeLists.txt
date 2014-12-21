project (Optimization)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${LOG4CXX_INCLUDE_DIR})
include_directories(${LBFGS_INCLUDE_DIR})
include_directories(${YAML_CPP_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR})
include_directories(${TBB_INCLUDE_DIR})

set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

set(LINK_DEPENDENCIES
  ${UTIL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LOG4CXX_LIBRARIES}
  ${ICU_LIBRARY}
  ${YAML_CPP_LIBRARY}
  ${LBFGS_LIB}
  Config
  Hypergraph
  dl
  )


# Optimization library
add_library(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/OptimizationProcedure.cpp
  ${PROJECT_SOURCE_DIR}/src/LearningRate.cpp
  ${PROJECT_SOURCE_DIR}/src/ExternalFeatHgPairs.cpp
  )
target_link_libraries(${PROJECT_NAME}
  ${LINK_DEPENDENCIES}
  )

sdl_rpath(${PROJECT_BINARY_DIR}/../CrfDemo)

# Executable for optimizing feature weights
set(TRAIN_EXE Optimize)
add_executable(${TRAIN_EXE}
  ${PROJECT_SOURCE_DIR}/src/OptimizeMain.cpp
  )
target_link_libraries(${TRAIN_EXE}
  ${LINK_DEPENDENCIES}
  ${PROJECT_NAME}
  )
