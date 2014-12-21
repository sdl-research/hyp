#in: TBB_ROOT

# Finds the TBB Library
#
#  TBB_ROOT           - Set to the path which contains include and lib for TBB
#  TBB_SHARED_ROOT    - Set to the optional path which contains include and lib for TBB
#  TBB_FOUND          - True if TBB found.
#  TBB_INCLUDE_DIRS   - Directory to include to get the TBB headers
#  TBB_LIBRARY        - Libraries to link against for TBB


# for header files.
unset(TBB_INCLUDE_DIR CACHE)
sdl_find_path(
  TBB_INCLUDE_DIR
  NAMES tbb/tbb.h
  PATHS ${TBB_ROOT}/include ${TBB_SHARED_ROOT}/include
  DOC "Include directory for TBB library")

# look for the library
set(tbb_lib_name tbb)
set(tbb_debug_lib_name ${tbb_lib_name})
set(tbbmalloc_lib_name tbbmalloc)
set(tbbmalloc_debug_lib_name ${tbbmalloc_lib_name})
set(tbbmalloc_proxy_lib_name tbbmalloc_proxy)
set(tbbmalloc_proxy_debug_lib_name ${tbbmalloc_proxy_lib_name})
if(WIN32) # If all works out we can move this to the Shared cmake folder and obsolete the Winodws/Linux versions
  # Allows VS to link debug builds against debug lib
  set(tbb_debug_lib_name tbb_debug)
  set(tbbmalloc_debug_lib_name tbbmalloc_debug)
  set(tbbmalloc_proxy_debug_lib_name tbbmalloc_proxy_debug)
endif()

unset(TBB_LIBRARY_RELEASE CACHE)
sdl_find_library(
  TBB_LIBRARY_RELEASE
  NAMES ${tbb_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBB")

unset(TBB_LIBRARY_DEBUG CACHE)
sdl_find_library(
  TBB_LIBRARY_DEBUG
  NAMES ${tbb_debug_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBB")

set(TBB_LIBRARY optimized ${TBB_LIBRARY_RELEASE} debug ${TBB_LIBRARY_DEBUG})

# look for the Scalable Memory Allocator library
unset(TBBMALLOC_LIBRARY_RELEASE CACHE)
sdl_find_library(
  TBBMALLOC_LIBRARY_RELEASE
  NAMES ${tbbmalloc_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBBMALLOC")

unset(TBBMALLOC_LIBRARY_DEBUG CACHE)
sdl_find_library(
  TBBMALLOC_LIBRARY_DEBUG
  NAMES ${tbbmalloc_debug_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBBMALLOC")

set(TBBMALLOC_LIBRARY optimized ${TBBMALLOC_LIBRARY_RELEASE} debug ${TBBMALLOC_LIBRARY_DEBUG})

unset(TBBMALLOC_PROXY_LIBRARY_RELEASE CACHE)
sdl_find_library(
  TBBMALLOC_PROXY_LIBRARY_RELEASE
  NAMES ${tbbmalloc_proxy_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBBMALLOC_PROXY")

unset(TBBMALLOC_PROXY_LIBRARY_DEBUG CACHE)
sdl_find_library(
  TBBMALLOC_PROXY_LIBRARY_DEBUG
  NAMES ${tbbmalloc_proxy_debug_lib_name}
  PATHS ${TBB_ROOT}/lib
  DOC "Libraries to link against for TBBMALLOC_PROXY")

set(TBBMALLOC_PROXY_LIBRARY optimized ${TBBMALLOC_PROXY_LIBRARY_RELEASE} debug ${TBBMALLOC_PROXY_LIBRARY_DEBUG})

if(TBB_INCLUDE_DIR AND TBB_LIBRARY)
  sdl_libpath(${TBB_LIBRARY_RELEASE})
  set(TBB_FOUND 1)
endif()

list(APPEND TBBMALLOC_LIBRARY ${TBBMALLOC_PROXY_LIBRARY})
