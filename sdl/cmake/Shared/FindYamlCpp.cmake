#-#  Try to find the yaml-cpp library
# Once done this will define:
#
# YAML_CPP_FOUND - system has yaml-cpp
# YAML_CPP_INCLUDE_DIR - the yaml-cpp include directory
# YAML_CPP_LIBRARY - yaml-cpp library

unset(YAML_CPP_INCLUDE_DIR CACHE)
sdl_find_path(YAML_CPP_INCLUDE_DIR yaml-cpp/yaml.h PATHS ${YAML_CPP_ROOT}/include ${YAML_CPP_SHARED_ROOT}/include)
#sdl_find_path(YAML_CPP_INCLUDE_DIR yaml-cpp/yaml.h PATHS ${YAML_CPP_ROOT}/include)

unset(YAML_CPP_LIBRARY_RELEASE CACHE)
sdl_find_library(YAML_CPP_LIBRARY_RELEASE yaml-cpp PATHS ${YAML_CPP_ROOT}/lib)
if(YAML_CPP_INCLUDE_DIR AND YAML_CPP_LIBRARY_RELEASE)
  set(YAML_CPP_FOUND 1)
  set(YAML_CPP_LIBRARY ${YAML_CPP_LIBRARY_RELEASE})
else()
  set(YAML_CPP_FOUND 0 CACHE BOOL "Not found yaml-cpp library")
  message(WARNING "Library 'yaml-cpp' not found (searched in ${YAML_CPP_ROOT}).")
endif()

mark_as_advanced(YAML_CPP_INCLUDE_DIR YAML_CPP_LIBRARY)
