#in: GSL_ROOT

# Finds the GSL Software Libraries headers
#
#  GSL_SHARED_ROOT    - Set to the path which contains files
#  GSL_FOUND          - True if found
#  GSL_INCLUDE_DIR    - Directory to include to get access to the headers

# for header files.
unset(GSL_INCLUDE_DIR CACHE)
find_path(
  GSL_INCLUDE_DIR
  NAMES gsl.h
  PATHS ${GSL_SHARED_ROOT}/include/
  NO_DEFAULT_PATH)

unset(GSL_FOUND CACHE)
if(GSL_INCLUDE_DIR)
  set(GSL_FOUND 1)
else()
  set(GSL_FOUND 0)
  message(WARNING "GSL not found in ${GSL_SHARED_ROOT}")
endif()
