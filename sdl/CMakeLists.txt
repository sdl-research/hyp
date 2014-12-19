



















    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel RelWithoutTbbmalloc."





































































# Increasing this will use more space for all kenlm states, so this should be as low as possible.


else()

endif()


# Run CMAKE with -DBUILD_KENLM to force re-building of the KenLM shared library. This option requires that the

if(BUILD_KENLM AND NOT BUILD_KENLM EQUAL 0)
  # Build and link against a custom KenLM

else()
  # Use prebuilt KenLM shared/dynamic libs
  xmt_set(BUILD_KENLM 0 "Use prebuilt KenLM (i.e., do not build KenLM)")
endif()






















































add_definitions(-DU_HAVE_STD_STRING=1)



















if(LINUX)
  message(STATUS "The target system is LINUX.")
endif()
if(ANDROID)
  message(STATUS "The target system is ANDROID.")
endif()
if(APPLE)
  message(STATUS "The target system is APPLE.")
endif()
if(WIN32)
  message(STATUS "The target system is WINDOWS.")




endif()












































if(ANDROID)

  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

  macro(find_host_path)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
    find_path(${ARGN})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  endmacro()
endif()













    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel RelWithoutTbbmalloc."



if(UNIX AND NOT APPLE AND NOT ANDROID)
  set(LINUX 1)
endif()




if(NOT ANDROID)




  enable_testing()
endif()
















































if(HADOOP_YARN)

  message(STATUS "running using HDP2.1 hadoop libraries")
else()

  message(STATUS "running using CDH3 hadoop libraries")
endif()













































  # Set Boost options




  set(Boost_COMPILER "-vc100")





























  # Without this, the compiler complains some boost methods fail to return a value
  add_definitions("/DBOOST_NO_UNREACHABLE_RETURN_DETECTION")


































































































































    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")






































  if(ANDROID)
    if("$ENV{ANDROID_STANDALONE_TOOLCHAIN}" STREQUAL "")
      message(FATAL_ERROR "Please define env. var ANDROID_STANDALONE_TOOLCHAIN")
    endif()
    set(ANDROID_SYS_ROOT "$ENV{ANDROID_STANDALONE_TOOLCHAIN}/sysroot")
    set(CMAKE_CXX_FLAGS "         -Wall -Wno-unused-variable -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wreturn-type -Wno-strict-aliasing -Wno-multichar --sysroot=${ANDROID_SYS_ROOT}")
  else(ANDROID)


  endif()







































  if(NOT ANDROID)



  if(LINUX)




    set(Boost_COMPILER "-gcc44")














  if(ANDROID)

    set(Boost_find_version 1.49)
    set(Boost_ADDITIONAL_VERSIONS "1.49" "1.49.0")
    set(Boost_NO_SYSTEM_PATHS 1)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_COMPILER "-gcc")


    set(LOG4CXX_LIB_DIR ${LOG4CXX_ROOT}/lib)
    set(LOG4CXX_LIB_DEBUG_DIR ${LOG4CXX_ROOT}/lib) # always use release for Android






  endif()













   set(LOG4CXX_LIB_DIR ${LOG4CXX_ROOT}/lib)





















































MARK_AS_ADVANCED(
  CMAKE_CXX_FLAGS_RELWITHOUTTBBMALLOC
  )

if(ANDROID)
  add_definitions(-DNLOG)
  MESSAGE(STATUS "Disabling logging for Android")


# Suppress warnings/errors for Production builds



else()

endif()

# CA-7791 Suppress source code locations in log messages; used when building in release mode for external customers



endif()



  if(ANDROID)

















  endif()











  set(LINK_DEPENDENCIES ${LINK_DEPENDENCIES} ${ICU_LIBRARIES})





MESSAGE(STATUS "TBB Include Directories: ${TBB_INCLUDE_DIR}")
if(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR ${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo" OR ${CMAKE_BUILD_TYPE} MATCHES "RelWithoutTbbmalloc")
  unset(TBBMALLOC_LIBRARY CACHE)






  find_package(Cryptopp)


  if(${CRYPTOPP_FOUND})
















MESSAGE(STATUS "Zeromq include directory: ${ZEROMQ_INCLUDE_DIR}")

if(ZEROMQ_FOUND)
  add_definitions(-DHAVE_ZMQ)
endif()

find_package(Svmtool)
MESSAGE(STATUS "Svmtool Library: ${SVMTOOL_LIBRARY}")
MESSAGE(STATUS "Svmtool Include: ${SVMTOOL_INCLUDE_DIR}")
if(SVMTOOL_FOUND)
  add_definitions(-DHAVE_SVMTOOL)
endif()

find_package(Liblinear)
MESSAGE(STATUS "Liblinear Library: ${LIBLINEAR_LIBRARY}")
MESSAGE(STATUS "Liblinear Include: ${LIBLINEAR_INCLUDE_DIR}")
if(LIBLINEAR_FOUND)
  add_definitions(-DHAVE_LIBLINEAR)
endif()

find_package(Tinyxmlcpp)
message(STATUS "TinyXML++ library: ${TINYXMLCPP_LIBRARY}")
message(STATUS "TinyXML++ include directory: ${TINYXMLCPP_INCLUDE_DIR}")



























    endif()




  message(STATUS "KenLM include directory: ${KENLM_INCLUDE_DIR}")



  find_package(KenLM)
  message(STATUS "KenLM libraries: ${KENLM_LIBRARIES}")
  message(STATUS "KenLM include directory: ${KENLM_INCLUDE_DIR}")

add_definitions(-DHAVE_KENLM)
set(HAVE_KENLM 1)


if(${LINUX})


  find_package(NPLM)
  message(STATUS "NPLM include dir: ${NPLM_INCLUDE_DIR}")
  message(STATUS "NPLM libs: ${NPLM_LIBRARIES}")


  message(STATUS "EIGEN include dir: ${EIGEN_INCLUDE_DIR}")
  message(STATUS "TCLAP include dir: ${TCLAP_INCLUDE_DIR}")
  add_definitions(-DHAVE_NPLM)
  set(HAVE_NPLM 1)
endif()

find_package(Protobuf REQUIRED)





    if(NOT ANDROID)



endif()



if(${HAVE_NPLM})

endif()











SET(boost_libs














  list(APPEND boost_libs
    unit_test_framework
    locale)








# Proceed to generate the normal all-inclusive Boost libraries

find_package(Boost ${Boost_find_version} REQUIRED COMPONENTS ${boost_libs})



# Create a list of Boost libraries that can be used to build shared libs; this list is essentially the same as the
# normal list with the exception of the unit_test_framework.
# Due to the Boost quirk described in https://svn.boost.org/trac/boost/ticket/5553, running boost unit-tests built in
# Debug mode against a dynamic shared library built in Release mode, will cause segfaults to occur: in order to avoid
# this, we simply avoid linking dynamic shared libs against the Boost unit-test framework.


  LIST(REMOVE_ITEM boost_libs_shared "unit_test_framework")








  elseif(NOT WIN32)






































if(NOT ANDROID)



endif()




MESSAGE(STATUS "Berkeley DB Libraries: ${DB_LIBRARIES}")





















find_package(CDB)
MESSAGE(STATUS "CDB Libraries: ${CDB_LIBRARIES}")
MESSAGE(STATUS "CDB Include Directories: ${CDB_INCLUDE_DIR}")











  message(STATUS "OpenSSL libraries for Hadoop: ${HADOOP_SSL_LIBRARY}, ${HADOOP_CRYPTO_LIBRARY}")
  message(STATUS "OpenSSL include path for Hadoop: ${HADOOP_SSL_INCLUDE_DIR}")
  set(_JAVA_PATHS "/usr/java/")
endif()

#
# JNI to work with DFS
#
find_package(JNI)

  set(JAVA_INCLUDE_PATH "$ENV{JAVA_HOME}\\include")
  set(JAVA_INCLUDE_PATH2 "$ENV{JAVA_HOME}\\include\\win32")
endif()
include_directories(${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
MESSAGE (STATUS "JNI header: ${JAVA_INCLUDE_PATH}")
MESSAGE (STATUS "JVM shared library: ${JAVA_JVM_LIBRARY}")

find_program(MAVEN_EXECUTABLE mvn ${MAVEN_ROOT} NO_DEFAULT_PATH)
if(NOT MAVEN_EXECUTABLE)
  MESSAGE(FATAL_ERROR "Maven was not found.")
endif(NOT MAVEN_EXECUTABLE)






























MESSAGE(STATUS "libLBFGS Libraries: ${LBFGS_LIB}")
MESSAGE(STATUS "libLBFGS Include Directories: ${LBFGS_INCLUDE_DIR}")




















# By default, create only the dynamic XMT lib. The user can override this

# - "Shared" -- (Default mode) xmtShell and related binaries will link dynamically to this
# - "Static" -- xmtShell and related binaries will link statically to this
# - "All" -- builds both libraries and links to the xmtShell and related binaries statically


else()

endif()

if(BUILD_KENLM)

endif()



















  DependencyParser








  MaltServer



  MorphAnalyzer














  SpellChecker








  Trustscore









if(LIBLINEAR_FOUND)
  list(APPEND subDirectories
    Preorderer
    )
endif()









    RuleExtractor












if(NOT ANDROID)

endif()








































































  if(NOT ANDROID)







  endif()








MESSAGE(STATUS "Unit test (check only) executables: ${testExecutables}")





























