project (Hypergraph)


if(OPENFST_FOUND)
  include_directories(${OPENFST_SRC_DIR})
  include_directories(${OPENFST_INCLUDE_DIR})
  sdl_compiled_setting(DHAVE_OPENFST 1)
  message(STATUS "Have openfst: -I${OPENFST_SRC_DIR} -I${OPENFST_INCLUDE_DIR}")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${LOG4CXX_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIR})

file(GLOB LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB HYP_SOURCES ${PROJECT_SOURCE_DIR}/src/Hyp*.cpp)
list(REMOVE_ITEM HYP_SOURCES ${PROJECT_SOURCE_DIR}/src/HypergraphBase.cpp)
foreach(item ${HYP_SOURCES})
  list(REMOVE_ITEM LIB_SOURCES ${item})
endforeach()
message(STATUS "Hypergraph lib sources: ${LIB_SOURCES}")

list(APPEND LINK_DEPENDENCIES ${LOG4CXX_LIBRARIES} ${UTIL_LIBRARIES})
if(NOT WIN32)
  set(LINK_DEPENDENCIES dl)
endif()
set(transform_DEPENDENCIES )

add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${LINK_DEPENDENCIES})

sdl_quiet_compiled_setting(GRAEHL_NAMED_SINGLE_MAIN 0) #mandatory

# 1 builds fewer tools (just Best Compose Inside Empty)
sdl_quiet_compiled_setting(SDL_MINIMAL_HYP_MAIN 0)

# Hyp.cpp includes all the other Hyp*.cpp sources now (single executable w/ argv[0] or argv[1] dispatch)
sdl_add_executable(hyp ${PROJECT_SOURCE_DIR} INCLUDE_ONLY src/Hyp.cpp)
#TODO: UTIL_LIBRARY_NAME is already in LINK_DEPENDENCIES
target_link_libraries(hyp ${PROJECT_NAME} ${LINK_DEPENDENCIES} Hypergraph ${UTIL_LIBRARY_NAME})
