






























































































































































function(xmt_add_library_explicit PROJ_NAME)
  if (WIN32)



  endif()
  add_library(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_library_explicit)

function(xmt_add_executable_explicit PROJ_NAME)
  if (WIN32)



  endif()
  add_executable(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_executable_explicit)




















































  # If the user specified a target name, use it; otherwise, assume the project-name
  set(TARGET_NAME ${ARGV0})
  if(NOT TARGET_NAME)
    set(TARGET_NAME ${PROJECT_NAME})
  endif()

  set_property(TARGET ${TARGET_NAME} PROPERTY PUBLIC_HEADER ${INCS})
  install(TARGETS ${TARGET_NAME}




    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/include/${TARGET_NAME}/include)
  install(TARGETS ${TARGET_NAME}




    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/xmt/include/${TARGET_NAME}/include)

























































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


