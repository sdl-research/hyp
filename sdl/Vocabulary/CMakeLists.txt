project (Vocabulary)

sdl_util_includes()

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARY_NAME} ${INDIRECT_Vocabulary_LIB_DEPENDENCIES})
