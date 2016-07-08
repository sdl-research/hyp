#in: ICU_ROOT

# Finds the International Components for Unicode (ICU) Library
#
#  ICU_ROOT	      - Set to the path which should be included in the search.
#  ICU_FOUND          - True if ICU found.
#  ICU_I18N_FOUND     - True if ICU's internationalization library found.
#  ICU_INCLUDE_DIRS   - Directory to include to get ICU headers
#                       Note: always include ICU headers as, e.g.,
#                       unicode/utypes.h
#  ICU_LIBRARIES      - Libraries to link against for the common ICU
#  ICU_I18N_LIBRARIES - Libraries to link against for ICU internationaliation
#                       (note: in addition to ICU_LIBRARIES)


# Look for the header file.
unset(ICU_INCLUDE_DIR CACHE)
sdl_find_path(
  ICU_INCLUDE_DIR
  NAMES unicode/utypes.h
  PATHS ${ICU_ROOT}/include
  DOC "Include directory for the ICU library"
  )

# Look for the library.
unset(ICU_LIBRARY CACHE)
sdl_find_library(
  ICU_LIBRARY
  NAMES icuuc
  PATHS ${ICU_ROOT}/lib
  DOC "Libraries to link against for the common parts of ICU"
  )
set(ICU_LIBRARY optimized ${ICU_LIBRARY} debug ${ICU_LIBRARY})

unset(ICUIO_LIBRARY CACHE)
sdl_find_library(
  ICUIO_LIBRARY
  NAMES icuio
  PATHS ${ICU_ROOT}/lib
  DOC "Libraries to link against for I/O facilities in ICU"
  )
set(ICUIO_LIBRARY optimized ${ICUIO_LIBRARY} debug ${ICUIO_LIBRARY})

unset(ICULE_LIBRARY CACHE)
sdl_find_library(
  ICULE_LIBRARY
  NAMES icule
  PATHS ${ICU_ROOT}/lib
  DOC "Libraries to link against for ICU"
  )
set(ICULE_LIBRARY optimized ${ICULE_LIBRARY} debug ${ICULE_LIBRARY})

unset(ICULX_LIBRARY CACHE)
sdl_find_library(
  ICULX_LIBRARY
  NAMES iculx
  PATHS ${ICU_ROOT}/lib
  DOC "Libraries to link against for ICU"
  )
set(ICULX_LIBRARY optimized ${ICULX_LIBRARY} debug ${ICULX_LIBRARY})

unset(ICUTU_LIBRARY CACHE)
sdl_find_library(
  ICUTU_LIBRARY
  NAMES icutu
  PATHS ${ICU_ROOT}/lib
  DOC "Libraries to link against for ICU"
  )
set(ICUTU_LIBRARY optimized ${ICUTU_LIBRARY} debug ${ICUTU_LIBRARY})

IF(WIN32)
  unset(ICUIN_LIBRARY CACHE)
  sdl_find_library(
    ICUIN_LIBRARY
    NAMES icuin
    PATHS ${ICU_ROOT}/lib
    DOC "Libraries to link against for ICU"
    )
  mark_as_advanced(ICUIN_LIBRARY)
ENDIF()

mark_as_advanced(ICU_LIBRARY)
mark_as_advanced(ICU_INCLUDE_DIR)
mark_as_advanced(ICUIO_LIBRARY)
mark_as_advanced(ICULE_LIBRARY)
mark_as_advanced(ICULX_LIBRARY)
mark_as_advanced(ICUTU_LIBRARY)

IF(NOT WIN32)
  unset(ICU_I18N_LIBRARY CACHE)
  sdl_find_library(
    ICU_I18N_LIBRARY
    NAMES icui18n
    PATHS ${ICU_ROOT}/lib
    DOC "Libraries to link against for ICU internationalization"
    )
  set(ICU_I18N_LIBRARY optimized ${ICU_I18N_LIBRARY} debug ${ICU_I18N_LIBRARY})

  unset(ICUDATA_LIBRARY CACHE)
  sdl_find_library(
    ICUDATA_LIBRARY
    NAMES icudata
    PATHS ${ICU_ROOT}/lib
    DOC "Libraries to link against for data structures in ICU"
    )
  set(ICUDATA_LIBRARY optimized ${ICUDATA_LIBRARY} debug ${ICUDATA_LIBRARY})
  mark_as_advanced(ICUDATA_LIBRARY)
  mark_as_advanced(ICU_I18N_LIBRARY)

ENDIF()


# Copy the results to the output variables.
if(ICU_INCLUDE_DIR AND ICU_LIBRARY)

  STRING(REGEX REPLACE "unicode/utypes.h$" "" ICU_INCLUDE_DIR ${ICU_INCLUDE_DIR})
  set(ICU_FOUND 1)
  set(ICU_LIBRARIES ${ICU_LIBRARY} ${ICUTU_LIBRARY} ${ICULE_LIBRARY} ${ICULX_LIBRARY} ${ICUIO_LIBRARY} ${ICUDATA_LIBRARY} ${ICU_I18N_LIBRARY})
  IF(WIN32)
	  set(ICU_LIBRARIES ${ICU_LIBRARIES} ${ICUIN_LIBRARY})
  ENDIF()
  set(ICU_INCLUDE_DIRS ${ICU_INCLUDE_DIR})
  set(ICU_VERSION 0)
  set(ICU_MAJOR_VERSION 0)
  set(ICU_MINOR_VERSION 0)
  if (EXISTS "${ICU_INCLUDE_DIR}/unicode/uvernum.h")
    FILE(READ "${ICU_INCLUDE_DIR}/unicode/uvernum.h" _ICU_VERSION_CONENTS)
  else()
    FILE(READ "${ICU_INCLUDE_DIR}/unicode/uversion.h" _ICU_VERSION_CONENTS)
  endif()

  STRING(REGEX REPLACE ".*#define U_ICU_VERSION_MAJOR_NUM ([0-9]+).*" "\\1" ICU_MAJOR_VERSION "${_ICU_VERSION_CONENTS}")
  STRING(REGEX REPLACE ".*#define U_ICU_VERSION_MINOR_NUM ([0-9]+).*" "\\1" ICU_MINOR_VERSION "${_ICU_VERSION_CONENTS}")

  set(ICU_VERSION "${ICU_MAJOR_VERSION}.${ICU_MINOR_VERSION}")
  if (ICU_I18N_LIBRARY)
    set(ICU_I18N_FOUND 1)
    set(ICU_I18N_LIBRARIES optimized ${ICU_I18N_LIBRARY} debug ${ICU_I18N_LIBRARY})
  else (ICU_I18N_LIBRARY)
    set(ICU_I18N_FOUND 0)
    set(ICU_I18N_LIBRARIES)
  endif (ICU_I18N_LIBRARY)
else(ICU_INCLUDE_DIR AND ICU_LIBRARY)
  set(ICU_FOUND 0)
  set(ICU_I18N_FOUND 0)
  set(ICU_LIBRARIES)
  set(ICU_I18N_LIBRARIES)
  set(ICU_INCLUDE_DIRS)
  set(ICU_VERSION)
  set(ICU_MAJOR_VERSION)
  set(ICU_MINOR_VERSION)
endif(ICU_INCLUDE_DIR AND ICU_LIBRARY)

IF(ICU_FOUND)
  IF( NOT ICU_FIND_QUIETLY )
    MESSAGE( STATUS "Found ICU header files in ${ICU_INCLUDE_DIRS}")
    MESSAGE( STATUS "Found ICU libraries: ${ICU_LIBRARIES}")
  ENDIF( NOT ICU_FIND_QUIETLY )
ELSE(ICU_FOUND)
 IF(ICU_FIND_REQUIRED)
  MESSAGE( FATAL_ERROR "Could not find ICU" )
 ELSE(ICU_FIND_REQUIRED)
  MESSAGE( STATUS "Optional package ICU was not found" )
 ENDIF(ICU_FIND_REQUIRED)
ENDIF(ICU_FOUND)
