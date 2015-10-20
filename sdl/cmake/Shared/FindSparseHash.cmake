#in: SPARSEHASH_ROOT

# Finds the SPARSEHASH Software Libraries headers
#
#  SPARSEHASH_SHARED_ROOT    - Set to the path which contains files
#  SPARSEHASH_FOUND          - True if found
#  SPARSEHASH_INCLUDE_DIR    - Directory to include to get access to the headers

# for header files.
unset(SPARSEHASH_INCLUDE_DIR CACHE)
find_path(
  SPARSEHASH_INCLUDE_DIR
  NAMES sparsehash/dense_hash_map
  PATHS ${SPARSEHASH_SHARED_ROOT}/include/
  NO_DEFAULT_PATH)

unset(SPARSEHASH_FOUND CACHE)
if(SPARSEHASH_INCLUDE_DIR)
  set(SPARSEHASH_FOUND 1)
else()
  set(SPARSEHASH_FOUND 0)
  message(WARNING "SPARSEHASH not found in ${SPARSEHASH_SHARED_ROOT}")
endif()
