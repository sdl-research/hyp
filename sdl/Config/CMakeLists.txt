project (Config)

sdl_util_includes()

sdl_add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR})

set(LINK_DEPENDENCIES
  ${YAML_CPP_LIBRARY}
  ${UTIL_LIBRARIES}
  )

target_link_libraries(${PROJECT_NAME} ${LINK_DEPENDENCIES})

sdl_install_targets()
