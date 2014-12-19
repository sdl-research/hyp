







































#use this instead of FIND_PATH - uses paths in arguments before system default
macro(xmt_find_path TO FILENAME)
  FIND_PATH(${TO} ${FILENAME} ${ARGN} NO_DEFAULT_PATH)
  FIND_PATH(${TO} ${FILENAME} ${ARGN})
endmacro(xmt_find_path)

#todo: how do i test this? - not using for now
macro(strip_suffix SUF VARNAME)
  STRING(REGEX REPLACE ${SUF}"$" "" ${SUF} ${${SUF}})
endmacro(strip_suffix)

macro(xmt_find_library TO)
  find_library(${TO} ${ARGN} NO_DEFAULT_PATH)
  find_library(${TO} ${ARGN})
endmacro(xmt_find_library)















































































  message(CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS})





function(xmt_add_library PROJ_NAME SOURCE_DIR)
  aux_source_directory("${SOURCE_DIR}/src" LW_PROJECT_SOURCE_FILES)
  file(GLOB_RECURSE INCS "*.hpp")
  file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
  source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})
  add_library(${PROJ_NAME} ${LW_PROJECT_SOURCE_FILES} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_library)


function(xmt_add_executable PROJ_NAME SOURCE_DIR)
  aux_source_directory("${SOURCE_DIR}/src" LW_PROJECT_SOURCE_FILES)
  file(GLOB_RECURSE INCS "*.hpp")
  file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
  source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})
  add_executable(${PROJ_NAME} ${LW_PROJECT_SOURCE_FILES} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_executable)

function(xmt_add_library_explicit PROJ_NAME)
  if (WIN32)
    file(GLOB_RECURSE INCS "*.hpp")
    file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
    source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})
  endif()
  add_library(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_library_explicit)

function(xmt_add_executable_explicit PROJ_NAME)
  if (WIN32)
    file(GLOB_RECURSE INCS "*.hpp")
    file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
    source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})
  endif()
  add_executable(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_executable_explicit)

function(xmt_add_test_executable PROJ_NAME SOURCE_DIR)
  aux_source_directory("${SOURCE_DIR}/test" LW_PROJECT_TEST_FILES)
  file(GLOB_RECURSE TEST_INCS "test/*.hpp")
  add_executable(${PROJ_NAME} ${LW_PROJECT_TEST_FILES} ${TEST_INCS})

endfunction(xmt_add_test_executable)
























macro(xmt_msvc_links NAME)
  if (WIN32)
    if (MSVC)
      set_target_properties(${NAME} PROPERTIES
        LINK_FLAGS_DEBUG
        ${VS_MULTITHREADED_DEBUG_DLL_IGNORE_LIBRARY_FLAGS})
      set_target_properties(${NAME} PROPERTIES
        LINK_FLAGS_RELWITHDEBINFO
        ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS})
      set_target_properties(${NAME} PROPERTIES
        LINK_FLAGS_RELEASE
        ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS})
      set_target_properties(${NAME} PROPERTIES
        LINK_FLAGS_MINSIZEREL
        ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS})
    endif()
  endif()
endmacro(xmt_msvc_links)

macro(xmt_install_targets)

  # If the user specified a target name, use it; otherwise, assume the project-name
  set(TARGET_NAME ${ARGV0})
  if(NOT TARGET_NAME)
    set(TARGET_NAME ${PROJECT_NAME})
  endif()

  set_property(TARGET ${TARGET_NAME} PROPERTY PUBLIC_HEADER ${INCS})
  install(TARGETS ${TARGET_NAME}
    CONFIGURATIONS Release
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/release/lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/release/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/release/bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/include/${TARGET_NAME}/include)
  install(TARGETS ${TARGET_NAME}
    CONFIGURATIONS Debug
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/debug/lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/debug/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/debug/bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/include/${TARGET_NAME}/include)
endmacro(xmt_install_targets)

#needs to be macro for SET to matter:
macro(xmt_rpath PATH)
  if (UNIX)
    SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-rpath -Wl,${PATH}")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,-rpath -Wl,${PATH}")
    #    message(STATUS "xmt_rpath link = ${CMAKE_CXX_LINK_FLAGS}")
  endif()
endmacro(xmt_rpath)

macro(xmt_libpath LIB)
  get_filename_component(RPATHDIR ${LIB} PATH )
  message(STATUS "xmt_libpath ${LIB} = ${RPATHDIR}")
  xmt_rpath(${RPATHDIR})
endmacro(xmt_libpath)












  aux_source_directory("${PROJECT_SOURCE_DIR}/test" LW_PROJECT_TEST_FILES)
  file(GLOB_RECURSE TEST_INCS "test/*.hpp")
  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")

  enable_testing()

  foreach(tsrc ${LW_PROJECT_TEST_FILES})
    GET_FILENAME_COMPONENT(texe ${tsrc} NAME)
    STRING(REGEX REPLACE ".cpp$" "" texe ${texe})
    STRING(REGEX REPLACE "^Test" "" tname ${texe})
    add_executable(${texe} ${tsrc} ${TEST_INCS} ${PROJECT_SOURCE_DIR})


















  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -Dmaven.project.build.directory=${work_dir} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package




function(xmt_maven_project_with_profile TARGET_NAME BINARY_DIR PROFILE_NAME)
  set(work_dir ${BINARY_DIR}/target)
  message(STATUS "Run mvn package: \"${MAVEN_EXECUTABLE}\" wth profile:\"${PROFILE_NAME}\"")

  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -P${PROFILE_NAME} -Dmaven.project.build.directory=${work_dir} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})





    -Dmaven.project.build.directory=${work_dir} -Dtest_Dir=${CMAKE_CURRENT_SOURCE_DIR}/src/test/unit









function(xmt_maven_project_with_profile_test TARGET_NAME BINARY_DIR PROFILE_NAME)
  xmt_maven_project_with_profile(${TARGET_NAME} ${BINARY_DIR} ${PROFILE_NAME})
  xmt_maven_test(${TARGET_NAME} ${BINARY_DIR})
endfunction(xmt_maven_project_with_profile_test)


  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")
  xmt_add_test_executable(${PROJECT_TEST_NAME} ${PROJECT_SOURCE_DIR})
  enable_testing()
  xmt_add_unit_tests(${PROJECT_NAME} ${PROJECT_TEST_NAME})


