unset(APR_UTIL_INCLUDE_DIR CACHE)
sdl_find_path(APR_UTIL_INCLUDE_DIR apu.h PATHS ${APR_UTIL_ROOT}/include/apr-1)

unset(APR_UTIL_LIBRARY CACHE)
sdl_find_library(APR_UTIL_LIBRARY aprutil-1 PATHS ${APR_UTIL_ROOT}/lib)

if(APR_UTIL_INCLUDE_DIR AND APR_UTIL_LIBRARY)
  set(APR_UTIL_FOUND 1)
  MESSAGE(STATUS "Found apr-util library in ${APR_UTIL_ROOT}")
else()
  set(APR_UTIL_FOUND 0 CACHE BOOL "Not found apr-util library")
  message(WARNING "Library 'apr-util' not found (searched in ${APR_UTIL_ROOT}).")
endif()

mark_as_advanced(APR_UTIL_INCLUDE_DIR APR_UTIL_LIBRARY)
