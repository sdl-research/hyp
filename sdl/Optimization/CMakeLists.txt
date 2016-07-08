project (Optimization)

sdl_util_includes()
include_directories(${LBFGS_INCLUDE_DIR})

#set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

set(LINK_DEPENDENCIES
  Hypergraph
  Vocabulary
  ${CONFIG_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LOG4CXX_LIBRARIES}
  ${ICU_LIBRARY}
  ${YAML_CPP_LIBRARY}
  ${LBFGS_LIB}
  )
if(UNIX)
list(APPEND LINK_DEPENDENCIES dl)
endif()

# Optimization library
 sdl_add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR} EXCLUDE ${PROJECT_SOURCE_DIR}/src/OptimizeMain.cpp)

target_link_libraries(${PROJECT_NAME}
  ${LINK_DEPENDENCIES}
  )

sdl_rpath(${PROJECT_BINARY_DIR}/../CrfDemo)

# Executable for optimizing feature weights
set(TRAIN_EXE Optimize)
sdl_add_exe(${TRAIN_EXE} src/OptimizeMain.cpp)
target_link_libraries(${TRAIN_EXE}
  ${LINK_DEPENDENCIES}
  ${PROJECT_NAME}
  )
