# UTF8_FOUND
# UTF8_DIR

unset(UTF8_DIR CACHE)
IF(APPLE)
  sdl_find_path(UTF8_DIR utf8.h ${UTF8_ROOT})
ELSE(APPLE)
  find_path(UTF8_DIR utf8.h ${UTF8_ROOT})
ENDIF(APPLE)

IF(UTF8_DIR)
 SET(UTF8_FOUND 1)
# STRING(REGEX REPLACE "utf8$" "" UTF8_DIR ${UTF8_DIR})
 MESSAGE(STATUS "Found UTF-8 CPP in ${UTF8_DIR}")
 include_directories(${UTF8_DIR})
# add_definitions(-DHAVE_UTF8)
ELSE()
 MESSAGE(STATUS "Not found: UTF-8 CPP")
ENDIF()
