project (HypDemo)

sdl_lwutil_includes()

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}.cpp
  )
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARIES} Hypergraph)
