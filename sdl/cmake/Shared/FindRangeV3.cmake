#in: RangeV3_ROOT

# Finds the RangeV3 Software Libraries headers
#
#  RangeV3_SHARED_ROOT    - Set to the path which contains files
#  RangeV3_FOUND          - True if found
#  RangeV3_INCLUDE_DIR    - Directory to include to get access to the headers

# for header files.
unset(RangeV3_INCLUDE_DIR CACHE)
find_path(
  RangeV3_INCLUDE_DIR
  NAMES range/v3/range.hpp
  PATHS ${RangeV3_SHARED_ROOT}/include/
  NO_DEFAULT_PATH)

unset(RangeV3_FOUND CACHE)
if(RangeV3_INCLUDE_DIR)
  set(RangeV3_FOUND 1)
else()
  set(RangeV3_FOUND 0)
  message(WARNING "RangeV3 not found in ${RangeV3_SHARED_ROOT}")
endif()
