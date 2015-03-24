project (HypDemo)

sdl_util_includes()

add_executable(HypDemo
  ${PROJECT_SOURCE_DIR}/src/HypDemo.cpp
  )
target_link_libraries(HypDemo ${UTIL_LIBRARIES} Hypergraph)

if(NOT WIN32)
  add_executable(HypSpeedTest
  ${PROJECT_SOURCE_DIR}/src/HypSpeedTest.cpp
  )
target_link_libraries(HypSpeedTest ${UTIL_LIBRARIES} Hypergraph)
endif()
