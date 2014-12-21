project (Config)

sdl_lwutil_includes()

sdl_add_library_explicit(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/Config.cpp
  ${PROJECT_SOURCE_DIR}/src/YAMLConfigProcessor.cpp)

set(LINK_DEPENDENCIES
  ${YAML_CPP_LIBRARY}
  ${UTIL_LIBRARIES}
  )

target_link_libraries(${PROJECT_NAME} ${LINK_DEPENDENCIES})
