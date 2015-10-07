#SET( letters "" "\;a" b c "d\;d" )
#JOIN("${letters}" ":" output)
#MESSAGE("${output}") # :;a:b:c:d;d
function(join VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

macro(sdl_add_library_named LIBNAME)
  sdl_add_library(${LIBNAME} "" INCLUDE_ONLY ${ARGN})
  set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME_RELEASE "${LIBNAME}")
  set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME_DEBUG "${LIBNAME}_debug")
  set_target_properties(${LIBNAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY lib)
  set_target_properties(${LIBNAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY lib)
  set_target_properties(${LIBNAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY lib)
endmacro()

# this will set -DNAME=VAL and add an entry to
# xmt --compiled-settings via scripts/compiledSettings.py
# -> xmt/CompiledSettings.cpp
#TODO: USAGE string arg to pass to compiledSettings.py (and structured char const* settings[][3] ?)
macro(sdl_exe_linker_flags)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ARGN}")
  message(STATUS "xmt exe-only linker flags += ${ARGN}")
endmacro()

macro(sdl_linker_flags)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ARGN}")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${ARGN}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ARGN}")
  message(STATUS "xmt linker flags += ${ARGN}")
endmacro()

macro(sdl_compiled_setting NAME VAL)
  message(STATUS ${NAME}=${VAL})
  list(APPEND COMPILED_SETTINGS_NAMES ${NAME})
  add_definitions(-D${NAME}=${VAL})
endmacro()

macro(sdl_set NAME VAL HELP)
  message(STATUS "${NAME}=${VAL}")
  set(${NAME} ${VAL} CACHE STRING ${HELP})
endmacro()

macro(sdl_quiet_compiled_setting NAME VAL)
  add_definitions(-D${NAME}=${VAL})
endmacro()

#use this instead of FIND_PATH - uses paths in arguments before system default
macro(sdl_find_path TO FILENAME)
  FIND_PATH(${TO} ${FILENAME} ${ARGN} NO_DEFAULT_PATH)
  FIND_PATH(${TO} ${FILENAME} ${ARGN})
endmacro(sdl_find_path)

#todo: how do i test this? - not using for now
macro(strip_suffix SUF VARNAME)
  STRING(REGEX REPLACE ${SUF}"$" "" ${SUF} ${${SUF}})
endmacro(strip_suffix)

macro(sdl_find_library TO)
  find_library(${TO} ${ARGN} NO_DEFAULT_PATH)
  find_library(${TO} ${ARGN})
endmacro(sdl_find_library)

macro(sdl_add_cflags)
  message(STATUS "(sdl_add_cflags: ${ARGN})")
  set(SDL_EXTRA_CFLAGS "${SDL_EXTRA_CFLAGS} ${ARGN}")
  add_definitions("${ARGN}")
endmacro()

macro(sdl_add_ldflags)
  message("(sdl_add_ldflags: ${ARGN})")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ARGN}")
endmacro()

macro(sdl_add_ldcflags)
  message("(sdl_ldcflag: ${ARGN})")
  sdl_add_ldflags(${ARGN})
  sdl_add_cflags(${ARGN})
endmacro()

macro(sdl_clang_stdlib c_stdc)
  set(SDL_CLANG_STDLIB_ARG "-l${c_stdc}++ -stdlib=lib${c_stdc}++")
  sdl_add_ldcflags(${SDL_CLANG_STDLIB_ARG})
endmacro()

# i tried setting these variables in a function using set(VAR "val" PARENT_SCOPE) per stackoverflow and it didn't work.
# so we use macros for setting globals
macro(sdl_enable_cpp11)
  set(SDL_CPP11 1)
  sdl_add_cflags("-std=c++11")
  if(UNIX)
    add_definitions("-D_GLIBCXX_USE_CXX11_ABI=0")
  endif()
  if (CMAKE_COMPILER_IS_GNUXX)
    sdl_add_cflags("-Wno-unused-function")
  endif()
  if (APPLE)
    if (${CMAKE_COMPILER_IS_CLANGXX})
      # sdl_clang_stdlib(c)
      # http://cplusplusmusings.wordpress.com/2012/07/05/clang-and-standard-libraries-on-mac-os-x/
      # but this will only work if you install newer clang + libc++ - not an old xcode version
    endif()
  endif()

  #TODO@JG: investigate performance / regression changes between USE_BOOST_UNORDERED_SET 0 and 1
  add_definitions("-DUSE_BOOST_UNORDERED_SET=1")
  # if we don't use =1, then we risk by regression differences due to different
  # iteration orders between c++11 and not, and between compiler versions'
  # unordered_ impls. however, boost's have slightly worse performance than
  # std::tr1 and std::
endmacro()

macro(sdl_enable_debug_info)
  if (UNIX OR ANDROID OR APPLE)
    set(SDL_EXTRA_CFLAGS "${SDL_EXTRA_CFLAGS} ${SDL_DEBUG_INFO_GCC_ARG}")
    add_definitions(${SDL_EXTRA_CFLAGS})
    message(STATUS "using debugging info: ${SDL_DEBUG_INFO_GCC_ARG}")
  endif()
endmacro()

macro(sdl_gcc_flag NEW_GCCFLAG)
  if (CMAKE_COMPILER_IS_GNUXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NEW_GCCFLAG}")
    message(STATUS "added gcc flags: ${NEW_GCCFLAG}")
  endif()
endmacro()

macro(sdl_gcc_cpp11_flag NEW_GCCFLAG)
  if (CMAKE_COMPILER_IS_GNUXX AND SDL_CPP11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NEW_GCCFLAG}")
    message(STATUS "added gcc c++11 flags: ${NEW_GCCFLAG}")
  endif()
endmacro()

macro(sdl_finish_cflags)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDL_EXTRA_CFLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${SDL_EXTRA_CFLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SDL_EXTRA_CFLAGS}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${SDL_EXTRA_CFLAGS}")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} -pg -static-libgcc")
  set(CMAKE_CXX_FLAGS_RELWITHOUTTBBMALLOC "${CMAKE_CXX_FLAGS_RELEASE}")

  message(CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS})
  message(CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG})
  message(CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE})
  message(CMAKE_CXX_FLAGS_RELWITHDEBINFO = ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endmacro()

function(sdl_add_library LIBRARY_NAME SOURCE_DIR)
  set(options SHARED INTERFACE)
  set(oneValueArgs)
  set(multiValueArgs EXCLUDE INCLUDE INCLUDE_ONLY)
  cmake_parse_arguments(sdl_add_library "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Collect available source file names
  aux_source_directory("${SOURCE_DIR}/src" LW_PROJECT_SOURCE_FILES)
  file(GLOB_RECURSE INCS "*.hpp")
  file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
  list(APPEND ALL_SOURCE_FILES ${LW_PROJECT_SOURCE_FILES} ${INCS} ${TEMPLATE_IMPLS})

  # Add only these included file names
  if(sdl_add_library_INCLUDE_ONLY)
    set(ALL_SOURCE_FILES ${INCS} ${TEMPLATE_IMPLS})
    list(APPEND ALL_SOURCE_FILES ${sdl_add_library_INCLUDE_ONLY})
  endif()

  # Add included file names
  if(sdl_add_library_INCLUDE)
    list(APPEND ALL_SOURCE_FILES ${sdl_add_library_INCLUDE})
  endif()

  # Remove excluded file names
  if(sdl_add_library_EXCLUDE)
    list(REMOVE_ITEM  ALL_SOURCE_FILES ${sdl_add_library_EXCLUDE})
  endif()

  source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})

  if(sdl_add_library_SHARED)
    add_library(${LIBRARY_NAME} SHARED ${ALL_SOURCE_FILES})
    sdl_msvc_links(${LIBRARY_NAME})
  elseif(sdl_add_library_INTERFACE)
    add_library(${LIBRARY_NAME} INTERFACE)
  else()
    add_library(${LIBRARY_NAME} ${ALL_SOURCE_FILES})
    sdl_msvc_links(${LIBRARY_NAME})
  endif()

endfunction(sdl_add_library)

function(sdl_add_executable EXECUTABLE_NAME SOURCE_DIR)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs EXCLUDE INCLUDE INCLUDE_ONLY)
  cmake_parse_arguments(sdl_add_executable "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Collect available source file names
  aux_source_directory("${SOURCE_DIR}/src" LW_PROJECT_SOURCE_FILES)
  file(GLOB_RECURSE INCS "*.hpp")
  file(GLOB_RECURSE TEMPLATE_IMPLS "${SOURCE_DIR}/src/*.ipp")
  list(APPEND ALL_SOURCE_FILES ${LW_PROJECT_SOURCE_FILES} ${INCS} ${TEMPLATE_IMPLS})

  # Add only these included file names
  if(sdl_add_executable_INCLUDE_ONLY)
    set(ALL_SOURCE_FILES ${INCS} ${TEMPLATE_IMPLS})
    list(APPEND ALL_SOURCE_FILES ${sdl_add_executable_INCLUDE_ONLY})
  endif()

  # Add included file names
  if(sdl_add_executable_INCLUDE)
    list(APPEND ALL_SOURCE_FILES ${sdl_add_executable_INCLUDE})
  endif()

  # Remove excluded file names
  if(sdl_add_executable_EXCLUDE)
    list(REMOVE_ITEM ALL_SOURCE_FILES ${sdl_add_executable_EXCLUDE})
  endif()

  source_group("Template Definitions" FILES ${TEMPLATE_IMPLS})
  add_executable(${EXECUTABLE_NAME} ${ALL_SOURCE_FILES})
  sdl_msvc_links(${EXECUTABLE_NAME})
endfunction(sdl_add_executable)

function(sdl_add_test_executable EXECUTABLE_NAME SOURCE_DIR)
  aux_source_directory("${SOURCE_DIR}/test" LW_PROJECT_TEST_FILES)
  file(GLOB_RECURSE TEST_INCS "test/*.hpp")
  add_executable(${EXECUTABLE_NAME} ${LW_PROJECT_TEST_FILES} ${TEST_INCS})
  #sdl_msvc_links(${EXECUTABLE_NAME})
endfunction(sdl_add_test_executable)

# macro so we can grab sdl_UTEST_DIR var later ? seems to work as function.
function(sdl_add_unit_test PROJECT_NAME TEST_EXE SUB_UNIT_TEST_NAME)
  add_test(${PROJECT_NAME}-${SUB_UNIT_TEST_NAME} ${TEST_EXE} --catch_system_errors --detect_memory_leaks --log_level=test_suite --run_test=${SUB_UNIT_TEST_NAME} --log_format=XML --log_sink=${sdl_UTEST_DIR}/${PROJECT_NAME}-testlog.xml)
endfunction(sdl_add_unit_test)

function(sdl_add_unit_tests PROJECT_NAME TEST_EXE)
  add_test(${PROJECT_NAME} ${TEST_EXE} --catch_system_errors --log_level=test_suite --log_format=XML --log_sink=${sdl_UTEST_DIR}/${PROJECT_NAME}-testlog.xml)
endfunction(sdl_add_unit_tests)

function(sdl_unit_tests_executable EXECUTABLE_NAME SOURCE_DIR)
  sdl_add_test_executable(${EXECUTABLE_NAME} ${SOURCE_DIR})
  sdl_add_unit_tests(${EXECUTABLE_NAME}-all ${EXECUTABLE_NAME})
endfunction(sdl_unit_tests_executable)

#TODO: smilar/redundant to sdl_project_test(s): sdl_unit_tests(${LINK_DEPENDENCIES} ${PROJECT_NAME})
macro(sdl_unit_tests)
  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")
  sdl_unit_tests_executable(${PROJECT_TEST_NAME} ${PROJECT_SOURCE_DIR})
  target_link_libraries(${PROJECT_TEST_NAME} ${ARGN} ${SDL_BOOST_TEST_LIBRARIES})
  enable_testing()
endmacro(sdl_unit_tests)


macro(sdl_msvc_links NAME)
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
endmacro(sdl_msvc_links)

macro(sdl_install_targets)

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
endmacro(sdl_install_targets)

#needs to be macro for SET to matter:
macro(sdl_rpath PATH)
  if (UNIX)
    SET(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-rpath -Wl,${PATH}")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,-rpath -Wl,${PATH}")
    #    message(STATUS "sdl_rpath link = ${CMAKE_CXX_LINK_FLAGS}")
  endif()
endmacro(sdl_rpath)

macro(sdl_libpath LIB)
  get_filename_component(RPATHDIR ${LIB} PATH )
  message(STATUS "sdl_libpath ${LIB} = ${RPATHDIR}")
  sdl_rpath(${RPATHDIR})
endmacro(sdl_libpath)

## this is the simplest way to add unit tests (where there's a single test/TestProjectName.cpp)
macro(sdl_project_test)
  # Unit tests:
  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")
  sdl_add_test_executable(${PROJECT_TEST_NAME} ${PROJECT_SOURCE_DIR})
  target_link_libraries(${PROJECT_TEST_NAME} ${PROJECT_NAME} ${LINK_DEPENDENCIES} ${SDL_BOOST_TEST_LIBRARIES})
  enable_testing()
  sdl_add_unit_tests(${PROJECT_NAME} ${PROJECT_TEST_NAME} ${ARGN})
endmacro(sdl_project_test)

macro(sdl_tests)
  aux_source_directory("${PROJECT_SOURCE_DIR}/test" LW_PROJECT_TEST_FILES)
  file(GLOB_RECURSE TEST_INCS "test/*.hpp")
  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")

  enable_testing()

  foreach(tsrc ${LW_PROJECT_TEST_FILES})
    GET_FILENAME_COMPONENT(texe ${tsrc} NAME)
    STRING(REGEX REPLACE ".cpp$" "" texe ${texe})
    STRING(REGEX REPLACE "^Test" "" tname ${texe})
    add_executable(${texe} ${tsrc} ${TEST_INCS} ${PROJECT_SOURCE_DIR})
    target_link_libraries(${texe} ${SDL_BOOST_TEST_LIBRARIES} ${LINK_DEPENDENCIES} ${UTIL_LIBRARIES} ${ARGN})
    #TODO: use sdl_add_unit_test macro?
    add_test(${PROJECT_NAME}-${tname} ${texe} "--catch_system_errors --detect_memory_leaks --log_level=test_suite --log_format=XML --log_sink=${sdl_UTEST_DIR}/${PROJECT_NAME}-${tname}.xml")
    # sdl_msvc_links(${PROJECT_NAME}-${tname})
  endforeach(tsrc)
  #sdl_msvc_links(${PROJECT_TEST_NAME})
  #Set Project & TestProject properties.
endmacro(sdl_tests)

## when there are multiple test/Test*.cpp
macro(sdl_project_tests)
  sdl_tests(${PROJECT_NAME} ${ARGN})
endmacro(sdl_project_tests)

function(sdl_maven_project TARGET_NAME BINARY_DIR)
  set(work_dir ${BINARY_DIR}/target)
  message(STATUS "Run mvn package: \"${MAVEN_EXECUTABLE}\"")

  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -Dmaven.project.build.directory=${work_dir} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endfunction(sdl_maven_project)

function(sdl_maven_project_with_profile TARGET_NAME BINARY_DIR PROFILE_NAME)
  set(work_dir ${BINARY_DIR}/target)
  message(STATUS "Run mvn package: \"${MAVEN_EXECUTABLE}\" wth profile:\"${PROFILE_NAME}\"")

  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -P${PROFILE_NAME} -Dmaven.project.build.directory=${work_dir} -Dmaven.project.build.directory.parent=${BINARY_DIR} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  set(INSTALLER ${TARGET_NAME}_installer)
  add_custom_target(${INSTALLER} ALL COMMAND ${MAVEN_EXECUTABLE} -Pcmake -Dmaven.project.build.directory=${work_dir} -Dmaven.project.build.directory.parent=${BINARY_DIR} -fae -B -q -f pom.xml -Dmaven.test.skip=true install
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${TARGET_NAME})
endfunction(sdl_maven_project_with_profile)

function(sdl_maven_test TARGET_NAME BINARY_DIR)
  set(work_dir ${BINARY_DIR}/target)
  add_test(Unit_Test_${TARGET_NAME} ${MAVEN_EXECUTABLE}
    -Dmaven.project.build.directory=${work_dir} -Dtest_Dir=${CMAKE_CURRENT_SOURCE_DIR}/src/test/unit
    -f ${CMAKE_CURRENT_SOURCE_DIR}/pom.xml test )
endfunction(sdl_maven_test)

function(sdl_maven_project_with_test TARGET_NAME BINARY_DIR)
  sdl_maven_project(${TARGET_NAME} ${BINARY_DIR})
  sdl_maven_test(${TARGET_NAME} ${BINARY_DIR})
endfunction(sdl_maven_project_with_test)


function(sdl_maven_project_with_profile_test TARGET_NAME BINARY_DIR PROFILE_NAME)
  sdl_maven_project_with_profile(${TARGET_NAME} ${BINARY_DIR} ${PROFILE_NAME})
  sdl_maven_test(${TARGET_NAME} ${BINARY_DIR})
endfunction(sdl_maven_project_with_profile_test)

macro(sdl_just_unit_tests)
  set(PROJECT_TEST_NAME "Test${PROJECT_NAME}")
  sdl_add_test_executable(${PROJECT_TEST_NAME} ${PROJECT_SOURCE_DIR})
  enable_testing()
  sdl_add_unit_tests(${PROJECT_NAME} ${PROJECT_TEST_NAME})
  target_link_libraries(${PROJECT_TEST_NAME} ${LINK_DEPENDENCIES} ${ARGN} ${SDL_BOOST_TEST_LIBRARIES})
endmacro(sdl_just_unit_tests)
