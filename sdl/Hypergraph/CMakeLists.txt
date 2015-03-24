project (Hypergraph)

sdl_quiet_compiled_setting(SDL_INLINE_HYP_CPPS 1)
sdl_quiet_compiled_setting(GRAEHL_NAMED_SINGLE_MAIN 0)

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
file(GLOB MAIN_SOURCES ${PROJECT_SOURCE_DIR}/src/Hyp*.cpp)
foreach(item ${MAIN_SOURCES})
  list(REMOVE_ITEM LIB_SOURCES ${item})
endforeach()

message(STATUS "Hypergraph lib sources: ${LIB_SOURCES}")

add_library(${PROJECT_NAME} ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${UTIL_LIBRARIES})

sdl_add_executable(Hyp ${PROJECT_SOURCE_DIR} INCLUDE_ONLY src/Hyp.cpp)
target_link_libraries(Hyp ${UTIL_LIBRARIES} Hypergraph)
