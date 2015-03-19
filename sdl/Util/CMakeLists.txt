project (Util)

sdl_util_includes()

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(${UTIL_LIBRARY_NAME} ${LIB_SOURCES})
target_link_libraries(${UTIL_LIBRARY_NAME} ${INDIRECT_UTIL_LIB_DEPENDENCIES})
