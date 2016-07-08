project (Hypergraph)


if(OPENFST_FOUND)
  include_directories(${OPENFST_SRC_DIR})
  include_directories(${OPENFST_INCLUDE_DIR})
  sdl_compiled_setting(DHAVE_OPENFST 1)
  message(STATUS "Have openfst: -I${OPENFST_SRC_DIR} -I${OPENFST_INCLUDE_DIR}")
endif()

sdl_util_includes()

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB HYP_SOURCES ${PROJECT_SOURCE_DIR}/src/Hyp*.cpp)
list(REMOVE_ITEM HYP_SOURCES ${PROJECT_SOURCE_DIR}/src/HypergraphBase.cpp)
foreach(item ${HYP_SOURCES})
  list(REMOVE_ITEM LIB_SOURCES ${item})
endforeach()
message(STATUS "Hypergraph lib sources: ${LIB_SOURCES}")

set(LINK_DEPENDENCIES Vocabulary ${UTIL_LIBRARIES})
if(NOT WIN32)
  #list(APPEND LINK_DEPENDENCIES dl)
endif()

add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LINK_DEPENDENCIES})

sdl_quiet_compiled_setting(GRAEHL_NAMED_SINGLE_MAIN 0)

# 1 builds fewer tools (just Best Compose Inside Empty)
sdl_quiet_compiled_setting(SDL_MINIMAL_HYP_MAIN 0)

# Hyp.cpp includes all the other Hyp*.cpp sources now (single executable w/ argv[0] or argv[1] dispatch)
sdl_add_exe(hyp src/Hyp.cpp)

show(LINK_DEPENDENCIES)
target_link_libraries(hyp ${PROJECT_NAME} ${LINK_DEPENDENCIES})
