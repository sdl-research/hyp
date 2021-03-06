unset(LEXERTL_DIR CACHE)
IF(APPLE)
  sdl_find_path(LEXERTL_DIR rules.hpp ${LEXERTL_ROOT}/lexertl)
ELSE()
  find_path(LEXERTL_DIR rules.hpp ${LEXERTL_ROOT}/lexertl)
ENDIF()

IF(LEXERTL_DIR)
 SET(LEXERTL_FOUND 1)
 MESSAGE(STATUS "Found lexertl in ${LEXERTL_DIR}/..")
 include_directories(${LEXERTL_DIR}/..)
ELSE()
 MESSAGE(STATUS "Not found: lexertl")
ENDIF()
