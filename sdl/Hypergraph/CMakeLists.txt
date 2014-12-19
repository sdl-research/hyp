project (Hypergraph)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${LOG4CXX_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR})

if(OPENFST_FOUND)
  include_directories(${OPENFST_SRC_DIR})
  include_directories(${OPENFST_INCLUDE_DIR})
  add_definitions(-DHAVE_OPENFST=1)
  message(STATUS "Have openfst: -DHAVE_OPENFST")
endif()

file(GLOB MAIN_SOURCES ${PROJECT_SOURCE_DIR}/src/Hyp*.cpp)

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

foreach(item ${MAIN_SOURCES})
  list(REMOVE_ITEM LIB_SOURCES ${item})
endforeach()

add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARIES})

SET(hygbins
  HypBest
  HypComplement
  HypCompose
  HypConcat
  HypConvertStrings
  HypDeterminize
  HypDraw
  HypEmpty
  HypFsmDraw
  HypGetString
  HypInside
  HypInvert
  HypIsolateStart
  HypProject
  HypPrune
  HypPruneToBest
  HypReverse
  HypReweight
  HypSamplePath
  HypTrie
  HypUnion
  HypWriteOpenFstText
  )

foreach(hygbin ${hygbins})
  add_executable(${hygbin} ${PROJECT_SOURCE_DIR}/src/${hygbin}.cpp)
  target_link_libraries(${hygbin} ${UTIL_LIBRARIES} Hypergraph)
endforeach()
