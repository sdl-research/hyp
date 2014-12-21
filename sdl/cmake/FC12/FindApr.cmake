unset(APR_INCLUDE_DIR CACHE)
sdl_find_path(APR_INCLUDE_DIR apr.h PATHS ${APR_ROOT}/include/include/apr-1)

unset(APR_LIBRARY CACHE)
sdl_find_library(APR_LIBRARY apr-1 PATHS ${APR_ROOT}/lib)

if(APR_INCLUDE_DIR AND APR_LIBRARY)
  set(APR_FOUND 1)
  MESSAGE(STATUS "Found apr library in ${APR_ROOT}")
else()
  set(APR_FOUND 0 CACHE BOOL "Not found apr library")
  message(WARNING "Library 'apr' not found (searched in ${APR_ROOT}).")
endif()

mark_as_advanced(APR_INCLUDE_DIR APR_LIBRARY)
