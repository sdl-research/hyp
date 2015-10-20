project (Vocabulary)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${SPARSEHASH_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${LOG4CXX_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR})

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARY_NAME} ${INDIRECT_Vocabulary_LIB_DEPENDENCIES})
