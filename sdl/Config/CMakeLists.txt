project(Config)

sdl_util_includes()

sdl_add_lib(${PROJECT_NAME}
src/Config.cpp
src/KeywordConfig.cpp
src/YAMLConfigProcessor.cpp
)

set(LINK_DEPENDENCIES
${UTIL_LIBRARIES}
${YAML_CPP_LIBRARY}
)

target_link_libraries(${PROJECT_NAME} ${LINK_DEPENDENCIES})

sdl_install_targets()
