project (HypDemo)

sdl_util_includes()

set(LINK_DEPENDENCIES Hypergraph Vocabulary ${UTIL_LIBRARIES})
add_executable(HypDemo
  ${PROJECT_SOURCE_DIR}/src/HypDemo.cpp
  )
sdl_target_libs(HypDemo ${LINK_DEPENDENCIES})

if(NOT WIN32)
  add_executable(HypSpeedTest
    ${PROJECT_SOURCE_DIR}/src/HypSpeedTest.cpp
    )
  sdl_target_libs(HypSpeedTest ${LINK_DEPENDENCIES})
endif()
