project (HypDemo)

sdl_util_includes()

sdl_add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARIES} Hypergraph)
