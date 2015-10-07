cmake_minimum_required(VERSION 3.1)
project (sdl)

include(SdlHelperFunctions.cmake)

if(DEFINED ENV{SDL_EXTERNALS_PATH})
  set(SDL_EXTERNALS_PATH $ENV{SDL_EXTERNALS_PATH})
else()
  message(FATAL_ERROR "Please set environment variable SDL_EXTERNALS_PATH")
endif()
string(REPLACE "\\" "/" SDL_EXTERNALS_PATH ${SDL_EXTERNALS_PATH})

# or cmake -DCMAKE_BUILD_TYPE=Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel RelWithoutTbbmalloc."
    FORCE)
endif()

if(SDL_BUILD_TYPE MATCHES "Development")
  set(SDL_PRODUCTION 0)
  set(SDL_DEVELOPMENT 1)
else()
  set(SDL_PRODUCTION 1)
  set(SDL_DEVELOPMENT 0)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  set(SDL_RELEASE_BUILD 1)
else()
  set(SDL_RELEASE_BUILD 0)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if(NOT DEFINED SDL_DEBUG_SYMBOLS)
  if (SDL_PRODUCTION)
    if(SDL_RELEASE_BUILD)
      set(SDL_DEBUG_SYMBOLS off)
    else()
      set(SDL_DEBUG_SYMBOLS on)
    endif()
  else()
    set(SDL_DEBUG_SYMBOLS on)
  endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Shared")
if(WIN32)
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Windows")
elseif(APPLE)
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/FC12")
elseif(ANDROID)
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Android")
elseif(UNIX)
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/FC12")
endif()
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

if(DEFINED SDL_EXTERNALS_PATH)
  set(SDL_SHARED_EXTERNALS_PATH "${SDL_EXTERNALS_PATH}/../Shared/cpp")
  message(STATUS "SDL_EXTERNALS_PATH=${SDL_EXTERNALS_PATH}")
else()
  message(FATAL_ERROR "Please set environment variable SDL_EXTERNALS_PATH")
endif()

option(OPEN_SOURCE_RELEASE "hyp open-source release" ON)


if (SDL_RELEASE_BUILD)
  add_definitions(-DEIGEN_NO_DEBUG)
else()
  add_definitions(-DEIGEN_NO_DEBUG)
endif()

# Use cmake -DSDL_FLOAT=32 or -DSDL_FLOAT=64
if(NOT DEFINED SDL_FLOAT)
  set(SDL_FLOAT 32)
endif()
sdl_compiled_setting(SDL_FLOAT ${SDL_FLOAT})


if(${SDL_FLOAT} MATCHES 32)
  message(STATUS "Using float precision")
elseif(${SDL_FLOAT} MATCHES 64)
  message(STATUS "Using double precision")
else ()
  message(STATUS "Unknown SDL_FLOAT value")
endif()

# speedup; see xmt/LexicalCast.hpp for rationale
add_definitions(-DBOOST_LEXICAL_CAST_ASSUME_C_LOCALE)
add_definitions(-DU_HAVE_STD_STRING)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR ${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
  set(SDL_DEBUG 1)
  set(SDL_RELEASE 0)
  sdl_quiet_compiled_setting(SDL_MEMSTATS 0) # detailed malloc pool info if run without tbb
else()
  set(SDL_DEBUG 0)
  set(SDL_RELEASE 1)
endif()

if(SDL_DEBUG)
  sdl_compiled_setting(SDL_VALGRIND 1)
else()
  # 0: *very*-slightly better performance (but spurious valgrind warnings)
  sdl_compiled_setting(SDL_VALGRIND 0)
endif()
if(OPEN_SOURCE_RELEASE)
  message(STATUS "OPEN_SOURCE_RELEASE")
  if (NOT APPLE)
    set(LINUX 1) #TODO: why isn't cmake setting this for us?
  endif()
endif()
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
  if(CMAKE_VERSION VERSION_GREATER "2.8.11")
    CMAKE_POLICY(SET CMP0022 OLD)
    ### you're supposed to set only one of LINK_INTERFACE_LIBRARIES and INTERFACE_LINK_LIBRARIES for each target, but we weren't so careful
  endif()
endif()

message(STATUS "CMAKE_CXX_COMPILER_ID is ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ARG1 is ${CMAKE_CXX_COMPILER_ARG1}")

set(CMAKE_CXX_COMMAND "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR "${CMAKE_CXX_COMMAND}" MATCHES "clang")
  set(USING_CLANG 1)
  set(CMAKE_COMPILER_IS_CLANGXX 1)
  message(STATUS "c++ is clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMMAND}" MATCHES 'g\\+\\+')
  set(USING_GCC 1)
  set(CMAKE_COMPILER_IS_GNUXX 1)
  message(STATUS "c++ is gnu")
  #  SET (CMAKE_AR "gcc-ar")
  #  SET (CMAKE_NM "gcc-nm")
  #  SET (CMAKE_RANLIB "gcc-ranlib")
else()
  set(USING_MSVC 1)
  set(CMAKE_COMPILER_IS_MSVC 1)
  message(STATUS "c++ is MSVC")
endif()
if("${CMAKE_CXX_COMMAND}" MATCHES "analyzer")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
  set(USING_SCAN_BUILD 1)
  message(STATUS "using scan-build c++analyzer from clang")
  set($ENV{CCC_CC} clang)
  set($ENV{CCC_CXX} clang++)
endif()
# later on we can strip symbols from release, or we can build without it (might be faster)
sdl_enable_debug_info()

if(UNIX)
  if(NOT CMAKE_COMPILER_IS_CLANGXX)
    #sdl_linker_flags(-shared-libgcc)

    ## supposedly shared-libgcc is required if you want to catch exceptions across
    ## library / program boundaries e.g. in main (without the exception simply
    ## triggering abort(). however, other documentation online suggests that this
    ## is a bogus practice and leads to a false sense of 'everything works fine'
    ## when running on a different linux than you built on.
  endif()
  if(SDL_GPROF)
    add_definitions("-pg")
    sdl_linker_flags(-pg)
    message(STATUS "Activated gprof flags")
  endif()
endif()
include_directories(${CMAKE_SOURCE_DIR})

if(ANDROID)
  set(CMAKE_FIND_ROOT_PATH ${SDL_EXTERNALS_PATH})
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

set(sdl_STAGE_PREFIX ${CMAKE_BINARY_DIR}/stage) #this is the stage/installation dir

include(GetGitRevisionDescription)


add_definitions(-DHAVE_CXX_STDHEADERS -DBOOST_ALL_NO_LIB -DTIXML_USE_TICPP)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel RelWithoutTbbmalloc."
    FORCE)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
  set(LINUX 1)
endif()

#
# Initialize unit testing.
#
if(NOT ANDROID)
  set(sdl_UTEST_DIR ${PROJECT_BINARY_DIR}/logs/unittests)
  if(NOT EXISTS ${sdl_UTEST_DIR})
    make_directory(${sdl_UTEST_DIR})
  endif()
  enable_testing()
endif()


set(UTF8_ROOT "${SDL_SHARED_EXTERNALS_PATH}/utf8")
set(YAML_CPP_ROOT "${SDL_EXTERNALS_PATH}/libraries/yaml-cpp-0.5.1")
set(YAML_CPP_SHARED_ROOT "${SDL_SHARED_EXTERNALS_PATH}/yaml-cpp-0.5.1")
if(UNIX)
  set(BZIP2_ROOT "${SDL_EXTERNALS_PATH}/libraries/bzip2")
endif()
if(ANDROID)
  set(YAML_CPP_ROOT "${SDL_EXTERNALS_PATH}/libraries/yaml-cpp-0.5.0")
endif()

if (WIN32)
  set(BZIP2_ROOT "${SDL_EXTERNALS_PATH}/libraries/bzip2-1.0.6")
endif()

set(TBB_ROOT "${SDL_EXTERNALS_PATH}/libraries/tbb-4.4.0")
set(TBB_SHARED_ROOT "${SDL_SHARED_EXTERNALS_PATH}/tbb-4.4.0")

if(UNIX AND NOT APPLE)
  # Apache Portable Runtime (used by Log4cxx)
  set(APR_ROOT "${SDL_EXTERNALS_PATH}/libraries/apr-1.4.2")
  find_package(Apr)

  set(APR_UTIL_ROOT "${SDL_EXTERNALS_PATH}/libraries/apr-util-1.3.10")
  find_package(AprUtil)
endif()


#
# Windows specific settings.
#
if(WIN32)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc /MP") #/MP and /Gm are mutually exclusive
  # By default, a project build enables the /Gm compiler option (incremental
  # builds) for debug builds, and disables it for release builds. Therefore,
  # the /MP compiler option is automatically disabled in debug builds because
  # it conflicts with the default /Gm compiler option. -
  # http://msdn.microsoft.com/en-us/library/bb385193.aspx

  # however, amos/manuel say that when we use cmake, an /MP option without any
  # /Gm option affects debug builds, too (and is faster if you have the
  # processors)

  sdl_linker_flags(/machine:x64)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")

  # For debugging on Windows, we recommend building a RelWithDebInfo build with all optimizations disabled. This is
  # effectively the same as building xmt in Debug mode; however, the advantage is that it allows you to link against
  # release mode 3rd party libs. Use a Debug build if you really want to link against the slower debug 3rdParty libs.
  # This option is similar to building with -DSDL_VALIDATION_BUILD, but with a few extra settings added.
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /Od /Ob0 /RTC1")

  # Enable TRACE statements for RelWIthDebInfo mode -- On Windows, this is the typical mode a developer will use when
  # writing code and debugging
  if(${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
    sdl_compiled_setting(SDL_ENABLE_TRACE_LOGGING 1)
  endif()

  if(SDL_VALIDATION_BUILD)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Od") # disable optimization for faster compile
    message(STATUS "SDL_VALIDATION_BUILD - faster (no optimization) build for Windows")
  else()
    sdl_linker_flags(/LTCG)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /GL") # some debugging info for actual release build
    message(STATUS "!SDL_VALIDATION_BUILD - optimized build for Windows")
  endif()

  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG")
  sdl_quiet_compiled_setting(_WINSOCKAPI_ 1) # but then db.h needs help - see BDBConfig.cpp
  #sdl_compiled_setting(_SECURE_SCL 0) # Causes end-iterator-dereference errors for debug Windows builds

  add_definitions(-DWIN32 -D_SCL_SECURE_NO_WARNINGS -D__TBB_NO_IMPLICIT_LINKAGE)
  # -DCRT_NO_DEPRECATE # could be used to get builtin vsnprintf if we want to try that

  # Set Boost options
  set(SDL_BOOST_MINOR 59)
  set(BOOST_ROOT "${SDL_EXTERNALS_PATH}/libraries/boost_1_${SDL_BOOST_MINOR}_0")
  set(BOOST_INCLUDEDIR "${SDL_SHARED_EXTERNALS_PATH}/boost_1_${SDL_BOOST_MINOR}_0/include")
  set(Boost_FIND_VERSION "1.${SDL_BOOST_MINOR}.0")
  set(Boost_COMPILER "-vc140")
  set(Boost_NO_SYSTEM_PATHS 1)
#  set(Boost_USE_STATIC_LIBS ON) # Enable if statically linking to boost
    add_definitions(-DBOOST_ALL_DYN_LINK) # Disable if statically linking to boost

  set(LOG4CXX_ROOT ${SDL_EXTERNALS_PATH}/libraries/log4cxx-0.10.0)
  set(ZLIB_PATH "${SDL_EXTERNALS_PATH}/libraries/zlib-1.2.8")


  set(VS_MULTITHREADED_DEBUG_DLL_IGNORE_LIBRARY_FLAGS
    "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:tbb_debug.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib")

  set(VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS
    "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
  add_definitions("/W3 /D_CRT_SECURE_NO_WARNINGS /wd4503 /wd4996 /wd4800")

  ## log4cxx: extern template instantiation
  add_definitions("/wd4231")

  ## openfst 'friend inline template'
  add_definitions("/wd4396")

  ##small_vector pod default constructor
  add_definitions("/wd4345")

  # Without this, the compiler complains some boost methods fail to return a value
  add_definitions("/DBOOST_NO_UNREACHABLE_RETURN_DETECTION")

  ##TODO: reenable the following:

  ## int cast lost of precision
  ##add_definitions("/wd4267")
  ## float cast loss of precision
  ##add_definitions("/wd4244")
  ##add_definitions("/wd4018")
  ## (people should really be forced to make explicit casts or fix their types)

endif()


set(SDL_NO_CLANG_OPTIMIZATIONS "-fcx-limited-range -fno-signaling-nans -fno-rounding-math")

# c++11
if(USING_GCC)
  #TODO: perf test - if no harm, keep c++11 threadsafe statics (we don't know
  #where in c++11 stdlib they're relied upon)
  # add_definitions("-fno-threadsafe-statics") # these are nice but MSVC doesn't provide them anyway
  add_definitions("-Wno-deprecated") # FC12/libraries/svmtool++/include/svmtool/tagger.h
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  message(STATUS "found gcc version ${GCC_VERSION}")
  if(GCC_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 5 or greater (you have '${CMAKE_CXX_COMPILER}', version=${GCC_VERSION}.")
  else ()
    set(SDL_HAVE_GCC_4_7 1)
    message(STATUS "have gcc 4.7 or higher")
    if(NOT GCC_VERSION VERSION_LESS 4.8)
      message(STATUS "have gcc 4.8 or higher")
      set(SDL_HAVE_GCC_4_8 1)

      if(LINUX)
        sdl_quiet_compiled_setting(UINT64_DIFFERENT_FROM_SIZE_T 0)
      else()
        sdl_quiet_compiled_setting(UINT64_DIFFERENT_FROM_SIZE_T 1)
      endif()
      #add_definitions(-Wextra) # enable only temporarily, since it's too much - see http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
      add_definitions(-fmax-errors=3) # quit after this many
      #add_definitions(-Werror) #TODO: boost 1.49 breaks this - see SpellChecker.cpp (boost thread interruption point)
      add_definitions(-Wno-unused-local-typedefs)
      #TODO: 1.49 lexical_cast false-positives this and we can't push/pop around lexical_cast.hpp because error occurs where value is used. but it's a valuable warning. so we should upgrade boost then reenable this
      add_definitions(-Wno-error=maybe-uninitialized)
      add_definitions(-Wno-type-limits)
      add_definitions(-Wno-maybe-uninitialized)
    endif()
    if(GCC_VERSION VERSION_LESS 4.9)
      add_definitions("-fno-tree-vectorize") # http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53533
    else()
      set(SDL_HAVE_GCC_4_9 1)
      # -static-libstdc++ (can't because tbb-4.3.1 is .so only)

      # sdl_exe_linker_flags("-static-libgcc") # (fails log4cxx)

      if (APPLE)
        if(GCC_VERSION VERSION_GREATER 5.0)
          # mac build isn't well supported currently (external lib builds weren't
          # maintained for it), and it seems like `brew install boost` and `brew
          # install gcc-5` don't understand _GLIBCXX_USE_CXX11_ABI, so likely a
          # manual build of all libs is needed
          include_directories(/usr/local/include/c++/5.1.0)
          sdl_enable_cpp11()
          sdl_exe_linker_flags("-static-libgcc")
          sdl_exe_linker_flags("-static-libstdc++")
          #-static
          # in case you want to quiet ld 'different visibility settings' warnings (makes exe larger?):
          # add_definitions("-fvisibility=hidden")
          # add_definitions("-fvisibility-inlines-hidden")
        endif()
      else()
        sdl_linker_flags("-lstdc++ -lm -fwhole-program")
        sdl_enable_cpp11()
      endif()
      #TODO: change to C++11 ABI once we have rebuilt all relevant (C++) libs
      sdl_compiled_setting(_GLIBCXX_USE_CXX11_ABI 0)
      if(OPEN_SOURCE_RELEASE)
        sdl_exe_linker_flags("-static-libgcc")
      else()
      endif()
    endif()
  endif()
elseif(USING_CLANG)
  add_definitions(-ftemplate-depth=255)
  add_definitions("-Wno-unknown-warning-option")
  add_definitions("-Wno-constant-logical-operand")
  add_definitions("-Wno-unused-local-typedef") # boost static assert triggers hundreds of these
  add_definitions("-Wno-deprecated-register")
  add_definitions("-Wno-deprecated-declarations")
  # note: llvm libc++ might not be compatible with some of our externals; you can use a libstdc++ from a recent GCC (e.g. 4.7) instead #-stdlib=libc++")
  if(USING_SCAN_BUILD)
    # sdl_enable_cpp11()
  endif()
  # sdl_enable_cpp11()
endif()


if(CMAKE_COMPILER_IS_CLANGXX)
  set(SDL_PTHREAD_LIB)
  set(SDL_PTHREAD_ARG)
  ### sanitizes are useful mostly at runtime - i.e. they're runtime checks that
  ### give you a crash instead of silent corruption. compare to valgrind

  # see http://clang.llvm.org/docs/UsersManual.html#controlling-code-generation

  # It is not possible to combine more than one of the -fsanitize=address,
  # -fsanitize=thread, and -fsanitize=memory checkers in the same program. The
  # -fsanitize=undefined checks can be combined with other sanitizers.

  if (USE_CLANG_SANITIZE_NO_RUNTIME)
    set(SDL_CLANG_SANITIZES "-fsanitize=undefined-trap -fsanitize-undefined-trap-on-error")
    #-fsanitize=undefined-trap: This includes all sanitizers included by
    #-fsanitize=undefined, except those that require runtime support. This group of
    #-sanitizers is intended to be used in conjunction with the
    #-fsanitize-undefined-trap-on-error flag. This includes all of the checks
    #-listed below other than unsigned-integer-overflow and vptr.
  else()
    #set(SDL_CLANG_SANITIZES "-fno-sanitize-recover")


    # -fsanitize-undefined-trap-on-error means don't use runtime (try this, and
    # -only enabling 'undefined' sanitize if you have linker errors

    # exit on first error instead of continuing: -fno-sanitize-recover - good idea
    # because sanitize has NO false positives unless you use inline ASM or
    # something strange. if you think it's not really a bug, you're probably
    # wrong!

    # -fsanitize=undefined' not allowed with '-fsanitize-undefined-trap-on-error' - traps shouldn't be used unless the runtime calls are undesirable (kernel devel?) for our purpose: don't use 'trap' options

    # both these are expensive (2x overhead or worse). TODO: can't use addr sanitizer when creating a shared lib either?
    if (CLANG_SANITIZE_address)
      set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=address")
    elseif (CLANG_SANITIZE_thread)
      set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=thread")
      #thread: data races (10x slowdown)
    elseif(CLANG_SANITIZE_undefined)
      set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=undefined")
    elseif(CLANG_SANITIZE_memory)
      set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=memory")
      #memory: uninitialized reads (3x slowdown)
    else()
      set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=address")
      #address: ref unallocated, double free, leaks (2x slowdown)
    endif()

    # also expensive: -fsanitize=integer - probably run one sanitize at a time
    # set(SDL_CLANG_SANITIZES "${SDL_CLANG_SANITIZES} -fsanitize=integer")
  endif()

  set(SDL_CLANG_COVERAGE_ARG "")
  if (CLANG_SANITIZE_coverage OR SDL_COVERAGE)
    message(STATUS "clang -fsanitize-coverage?")
    #set(SDL_CLANG_COVERAGE_ARG "-fsanitize-coverage=edge") #TODO: doesn't work? clang 3.6.1
    #-fsanitize-coverage=func -fsanitize-coverage=8bit-counters
    #http://clang.llvm.org/docs/SanitizerCoverage.html
  endif()

  add_definitions(-Qunused-arguments)
  add_definitions("-Wno-c++11-extensions -fshow-column -fshow-source-location -fcaret-diagnostics -fdiagnostics-fixit-info -fdiagnostics-parseable-fixits -fdiagnostics-print-source-range-info -fdiagnostics-show-option")
  if(USE_CLANG_ANALYZER)
    add_definitions("-Xanalyzer -analyzer-disable-checker=core.NullDereference")
    message(INFO "using clang (static) analyzer, which might slow compilation - disable USE_CLANG_ANALYZER cmake option if you want full speed")
  endif()
  if (USE_CLANG_SANITIZE)
    add_definitions("-fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls")
    message(INFO "using clang sanitize, which might have significant (2-3x) runtime overhead - disable USE_CLANG_SANITIZE cmake option if you want full speed")
    message(INFO "using clang sanitizes: ${SDL_CLANG_SANITIZES}")
    add_definitions("${SDL_CLANG_SANITIZES}")
    add_definitions("${SDL_CLANG_COVERAGE_ARG}")
    #${SDL_CLANG_SANITIZES}
    sdl_linker_flags(${SDL_CLANG_SANITIZES})
    SET(CMAKE_LINKER "clang++") # needed for sanitizes

    #  When linking shared libraries, the AddressSanitizer run-time is not
    #  linked, so -Wl,-z,defs may cause link errors (don’t use it with
    #  AddressSanitizer).

    set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
  endif()
  #http://comments.gmane.org/gmane.comp.compilers.clang.devel/9229

  # goal of libc++ - http://libcxx.llvm.org/ - ABI compatibility with gcc's
  # libstdc++ for some low-level features such as exception objects, rtti and
  # memory allocation.
  if(APPLE)
    # sdl_clang_stdlib(c)
    # libc++ is not yet fully working on linux: http://libcxx.llvm.org/results.Linux.html
    sdl_clang_stdlib(stdc)
  else()
    sdl_clang_stdlib(stdc)
  endif()
else()
  set(SDL_PTHREAD_LIB pthread)
  set(SDL_PTHREAD_ARG -pthread)
endif()


if(WIN32)
  set(SRILM_ROOT "${SDL_EXTERNALS_PATH}/libraries/srilm-1.7.0")
endif()

# Use cmake -DSDL_HAVE_BUILTIN_POPCNT=0 for older CPUs or -DSDL_HAVE_BUILTIN_POPCNT=1 newer CPUs
if(NOT DEFINED SDL_HAVE_BUILTIN_POPCNT)
  set(SDL_HAVE_BUILTIN_POPCNT 0) # Default this setting to off for now to match minimum CT & ETS CPU requirements
endif()
sdl_compiled_setting(HAVE_BUILTIN_POPCNT ${SDL_HAVE_BUILTIN_POPCNT})
# -mpopcnt gives 'population count' i.e. number of set bits - might require corei7 - check cluster machines!

if(UNIX)
  sdl_quiet_compiled_setting(HAVE_BUILTIN_CLZ 1)
  if (NOT CMAKE_COMPILER_IS_CLANGXX)
    if (NOT APPLE)
      sdl_linker_flags(-Wl,--no-undefined)
    endif()
  endif()
  if(ANDROID)
    if("$ENV{ANDROID_STANDALONE_TOOLCHAIN}" STREQUAL "")
      message(FATAL_ERROR "Please define env. var ANDROID_STANDALONE_TOOLCHAIN")
    endif()
    set(ANDROID_SYS_ROOT "$ENV{ANDROID_STANDALONE_TOOLCHAIN}/sysroot")
    set(CMAKE_CXX_FLAGS "         -Wall -Wno-unused-variable -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wreturn-type -Wno-strict-aliasing -Wno-multichar --sysroot=${ANDROID_SYS_ROOT}")
  else()
    set(SDL_CC_TUNE_ARGS "-march=core2 -mtune=generic")
    if(LINUX)
      set(SDL_CC_TUNE_ARGS "-march=native")
    endif()
    if(APPLE)
      if (SDL_HAVE_GCC_4_6)
        set(SDL_CC_TUNE_ARGS "-march=corei7")
      else()
        set(SDL_CC_TUNE_ARGS "-march=core2 -mtune=generic")
      endif()
      #  if (SDL_HAVE_GCC_4_9)
      #    set(SDL_CC_TUNE_ARGS "-march=sandybridge -mtune=sandybridge")
      #  else()
      #    set(SDL_CC_TUNE_ARGS "-march=corei7-avx")
      # endif()
    endif()
    set(CMAKE_CXX_FLAGS "${SDL_PTHREAD_ARG} -fPIC -Wall -Wno-unused-variable -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wreturn-type -Wno-strict-aliasing -Wno-multichar ${SDL_CC_TUNE_ARGS}")
  endif()

  set(SDL_Og "-O0")
  set(SDL_O3 "-O3")
  if(NOT APPLE)
    set(SDL_CFLAGS_ALL "-fopenmp")
  endif()
  if(SDL_HAVE_GCC_4_8)
    #add_definitions("-Wno-delete-non-virtual-dtor")
    add_definitions("-ftemplate-backtrace-limit=0")
  endif()
  set(SDL_UNSAFE_MATH "-funsafe-math-optimizations")
  if (SDL_HAVE_GCC_4_7)
    set(SDL_TESTED_RELEASE_OPTIMIZATIONS "-fassociative-math -fno-signed-zeros -fno-trapping-math -fno-math-errno -freciprocal-math")
  else()
    set(SDL_TESTED_RELEASE_OPTIMIZATIONS "-fno-trapping-math -fno-math-errno")
  endif()
  if (NOT USING_CLANG)
    set(SDL_TESTED_RELEASE_OPTIMIZATIONS "${SDL_TESTED_RELEASE_OPTIMIZATIONS} ${SDL_NO_CLANG_OPTIMIZATIONS}")
    if (SDL_HAVE_GCC_4_8)
      #set(SDL_O3 "-Ofast") # adds only -ffinite-math-only but breaks some release-mode unit tests on nan
      set(SDL_O3_ADDL "-fcx-limited-range -funsafe-math-optimizations")
      set(SDL_O3 "-O3 --param allow-store-data-races=1 ${SDL_O3_ADDL}")
    endif()
  endif()
  #-fmerge-all-constants violates separate C++ address for each constant but that's fine by us
  set(SDL_ALIGN_LOOPS_ETC_TO "1")
  # 1 means don't align - icache/noop vs align benefit tradeoff; 0 means let gcc
  # choose (supposedly bad in some cases - might pad only if #padding <10; 16 is
  # the most favorable possible for the CPU but does waste some space)
  # align-loops=16 depends - helpful if loop executes enough times
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(SDL_RELEASE_OPTIMIZATIONS "-fomit-frame-pointer ${SDL_TESTED_RELEASE_OPTIMIZATIONS}")
  else()
    set(SDL_RELEASE_OPTIMIZATIONS "-fno-keep-static-consts -fmerge-all-constants -DNDEBUG -DEIGEN_NO_DEBUG -falign-functions=${SDL_ALIGN_LOOPS_ETC_TO} -falign-loops=${SDL_ALIGN_LOOPS_ETC_TO} -falign-jumps=${SDL_ALIGN_LOOPS_ETC_TO} -falign-labels=1 -fomit-frame-pointer ${SDL_TESTED_RELEASE_OPTIMIZATIONS}")
  endif()
  #align-labels=1 is good because you need to avoid noop cost.
  if (NOT SDL_DEBUG_SYMBOLS)
    if(CMAKE_BUILD_TYPE MATCHES "ReleaseXXX")
      # apparently doesn't work for us (fPIC or cmake isn't using the right gcc
      # tools or we didn't build gcc 5 right)? see
      # https://gcc.gnu.org/wiki/LinkTimeOptimizationFAQ (/usr/bin/ar segfault via /local/gcc/bin/gcc-ar)
      sdl_linker_flags("-flto")
      add_definitions("-flto")
      MESSAGE(INFO "Release build with !SDL_DEBUG_SYMBOLS - trying -flto (link time optimization")
    endif()
  endif()

  set(CMAKE_CXX_FLAGS_DEBUG "${SDL_Og} ${SDL_DEBUG_INFO_GCC_ARG} ${SDL_CFLAGS_ALL} ${SDL_UNSAFE_MATH}")

  set(CMAKE_CXX_FLAGS_RELEASE "${SDL_O3} ${SDL_CFLAGS_ALL} -Wuninitialized ${SDL_RELEASE_OPTIMIZATIONS} ${SDL_UNSAFE_MATH}")

  sdl_linker_flags(${SDL_UNSAFE_MATH})


  #CPU:
  if(NOT ANDROID)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mfpmath=sse")
  endif()

  #set(Boost_DEBUG 1)
  #set(Boost_DETAILED_FAILURE_MSG 1)

  if(LINUX)
    set(Boost_NO_SYSTEM_PATHS 1)
    set(SDL_BOOST_MINOR 59)
    set(Boost_COMPILER "-gcc51")
    set(Boost_FIND_VERSION "1.${SDL_BOOST_MINOR}.0")
    set(BOOST_ROOT "${SDL_EXTERNALS_PATH}/libraries/boost_1_${SDL_BOOST_MINOR}_0")
    set(BOOST_INCLUDEDIR "${SDL_SHARED_EXTERNALS_PATH}/boost_1_${SDL_BOOST_MINOR}_0/include")
    set(Boost_USE_STATIC_LIBS OFF)
    set(LOG4CXX_ROOT ${SDL_EXTERNALS_PATH}/libraries/log4cxx-0.10.0)
    set(LOG4CXX_LIB_DIR ${LOG4CXX_ROOT}/lib)
    set(LOG4CXX_LIB_DEBUG_DIR ${LOG4CXX_ROOT}/libd)
    set(ICU_ROOT "${SDL_EXTERNALS_PATH}/libraries/icu-55.1")


    if(CMAKE_COMPILER_IS_GNUCXX)
      #    sdl_exe_linker_flags(-static -static-libgcc -static-libstdc++)
      # probably not safe to run reseulting exe across different linux/libc versions
    else()
      #clang
      add_definitions("-Wno-mismatched-tags -Wno-gnu")

      #tinyxml
      #NOTE: -Woverloaded-virtual is a VERY valuable warning, which is unfortunately useless given the amount of externals lib violations that show up in the compile. try compiling on windows; I've selectively disabled an equivalent warning for some of the externals.
      add_definitions("-Wno-overloaded-virtual")
      #boost:
      add_definitions("-Wunneeded-internal-declaration -Wno-bool-conversions -Wno-attributes -Wno-char-subscripts")
      # kenlm
      add_definitions("-Wno-unused-function")
    endif()

  endif() # APPLE

  if(NOT ANDROID)
    set(BOOST_INCLUDEDIR "${SDL_SHARED_EXTERNALS_PATH}/boost_1_${SDL_BOOST_MINOR}_0/include")
  endif()

endif() # UNIX

MARK_AS_ADVANCED(
  CMAKE_CXX_FLAGS_RELWITHOUTTBBMALLOC
  )

if(ANDROID)
  add_definitions(-DNLOG)
  MESSAGE(STATUS "Disabling logging for Android")
endif()

# Suppress warnings/errors for Production builds
if(SDL_SUPPRESS_IGNORABLE_WARNINGS)
  MESSAGE(STATUS "SDL_SUPPRESS_IGNORABLE_WARNINGS: Suppressing ignorable warnings and errors log messages")
  sdl_compiled_setting(SDL_SUPPRESS_IGNORABLE_WARNINGS 1)
else()
  sdl_compiled_setting(SDL_SUPPRESS_IGNORABLE_WARNINGS 0)
endif()

# CA-7791 Suppress source code locations in log messages; used when building in release mode for external customers
if(SDL_SUPPRESS_SOURCE_LOCATION)
  MESSAGE(STATUS "SDL_SUPPRESS_SOURCE_LOCATION: Suppressing source code location in log messages")
  sdl_compiled_setting(SDL_SUPPRESS_SOURCE_LOCATION 1)
endif()

# debugging flags:
if (SDL_DEBUG_SYMBOLS)
  if(ANDROID)
    set(SDL_DEBUG_INFO_GCC_ARG "-ggdb")
    #supposedly only -ggdb works for android.
  else()
    if(APPLE)
      set(SDL_DEBUG_INFO_GCC_ARG "-g")
    elseif(UNIX)
      if(SDL_COVERAGE)
        message(STATUS "SDL_COVERAGE: --coverage")
        if(USING_CLANG)
          set(SDL_CLANG_PROFILE "-fprofile-instr-generate -fcoverage-mapping")
          set(SDL_CLANG_PROFILE "--profile")
          add_definitions("${SDL_CLANG_PROFILE}")
          sdl_exe_linker_flags("${SDL_CLANG_PROFILE}")
          #          sdl_exe_linker_flags("-lprofile_rt")
        else()
          add_definitions("--coverage")
          #add_definitions("-fprofile-arcs -ftest-coverage")
          sdl_linker_flags("--coverage")
        endif()
      endif()
      # dwarf-3 or 4 should do a better job dealing with locals whose address changes
      # from one part of a fn to the next
      if(SDL_HAVE_GCC_4_7)
        set(SDL_DEBUG_INFO_GCC_ARG "-gdwarf-4")
        #set(SDL_DEBUG_INFO_GCC_ARG "-g")
      else()
        #set(SDL_DEBUG_INFO_GCC_ARG "-gdwarf-3")
        #dwarf-3 seems to create 500mb executable xmt and 1.0gb executable libxmt (they're normal size once stripped)
        set(SDL_DEBUG_INFO_GCC_ARG "-g")
      endif()
    endif()
  endif()
  add_definitions("${SDL_DEBUG_INFO_GCC_ARG}")
endif()

# 0mq zhelpers.h annoyance
if(CMAKE_COMPILER_IS_GNUXX)
  add_definitions(-Wno-unused-function)
endif()

find_package(ICU)
MESSAGE(STATUS "ICU Libraries: ${ICU_LIBRARIES}")
if(WIN32)
  set(LINK_DEPENDENCIES ${LINK_DEPENDENCIES} ${ICU_LIBRARIES})
endif()
sdl_quiet_compiled_setting(HAVE_ICU 1)

find_package(TBB)
MESSAGE(STATUS "TBB Library: ${TBB_LIBRARY}")
MESSAGE(STATUS "TBB Include Directories: ${TBB_INCLUDE_DIR}")
if(${CMAKE_BUILD_TYPE} MATCHES "Debug" OR ${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo" OR ${CMAKE_BUILD_TYPE} MATCHES "RelWithoutTbbmalloc")
  unset(TBBMALLOC_LIBRARY CACHE)
  set(TBBMALLOC_LIBRARY)
  MESSAGE(STATUS "Disabling TBB malloc: TBB_LIBRARY=${TBB_LIBRARY} TBBMALLOC_LIBRARY=${TBBMALLOC_LIBRARY}")
endif()



# Boost
set(Boost_USE_STATIC_RUNTIME OFF)
SET(boost_libs
  regex
  serialization
  system
  filesystem
  program_options
  date_time
  iostreams
  timer
  chrono
  thread
  random
  )

if(NOT ANDROID)
  list(APPEND boost_libs
    unit_test_framework
    locale)
endif()

if(WIN32)
#  list(APPEND boost_libs exception) # Enable if statically linking to boost
  list(APPEND boost_libs zlib) # Disable if statically linking to boost
endif()

set(boost_libs_shared ${boost_libs})

# Proceed to generate the normal all-inclusive Boost libraries
message(STATUS "looking for Boost version ${Boost_FIND_VERSION} compiler ${Boost_COMPILER}")
find_package(Boost ${Boost_FIND_VERSION} REQUIRED COMPONENTS ${boost_libs})

MESSAGE(STATUS "SDL_BOOST_MINOR ${SDL_BOOST_MINOR} Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS}")

#if 1, attempt to use unit_test_framework dylib, but provide our own main()
set(SDL_TRY_QUICK_EXIT_UNIT_TEST 1)
set(SDL_BOOST_TEST_LIBRARIES "")

# Create a list of Boost libraries that can be used to build shared libs; this
# list is essentially the same as the normal list with the exception of the
# unit_test_framework.  Due to the Boost quirk described in
# https://svn.boost.org/trac/boost/ticket/5553, running boost unit-tests built
# in Debug mode against a dynamic shared library built in Release mode, will
# cause segfaults to occur (reported in windwos but feared in general)
message(STATUS "${boost_libs_shared}")
if(WIN32)
  set(SDL_UNIT_TEST_MAIN 0)

  LIST(REMOVE_ITEM boost_libs_shared "unit_test_framework")
  #see http://www.boost.org/doc/libs/1_57_0/libs/test/doc/html/utf/compilation.html#utf.flag.alt-init-api
elseif(SDL_TRY_QUICK_EXIT_UNIT_TEST)
  set(SDL_BOOST_TEST_LIBRARIES "${Boost_LIBRARY_DIRS}/libboost_unit_test_framework${Boost_COMPILER}-mt-1_${SDL_BOOST_MINOR}.so")
  set(SDL_UNIT_TEST_MAIN 1)
  # Shared lib version of Boost.Test does not include main for portability
  # reasons, but if you -DBOOST_TEST_DYN_LINK it gets pulled in:
  if (SDL_UNIT_TEST_MAIN)
    add_definitions(-DBOOST_TEST_DYN_LINK)
  else()
    add_definitions(-DBOOST_TEST_DYN_LINK)
    MESSAGE(STATUS "BOOST_TEST_DYN_LINK")
  endif()
else()
  set(SDL_UNIT_TEST_MAIN 0)
  LIST(REMOVE_ITEM boost_libs_shared "unit_test_framework")
  LIST(REMOVE_ITEM boost_libs_shared "unit_test_framework")

  add_library(boost_unit_test_framework STATIC IMPORTED)
  list(APPEND Boost_LIBRARIES boost_unit_test_framework)
  if(APPLE)
    set(BOOST_UNIT_TEST_FRAMEWORK_LIB_STATIC "${Boost_LIBRARY_DIRS}/libboost_unit_test_framework${Boost_COMPILER}-mt.a")
  else()
    if(SDL_DEBUG)
      set(BOOST_UNIT_TEST_FRAMEWORK_LIB_STATIC ${Boost_LIBRARY_DIRS}/libboost_unit_test_framework${Boost_COMPILER}-mt-d-1_${SDL_BOOST_MINOR}.a)
    else()
      set(BOOST_UNIT_TEST_FRAMEWORK_LIB_STATIC ${Boost_LIBRARY_DIRS}/libboost_unit_test_framework${Boost_COMPILER}-mt-1_${SDL_BOOST_MINOR}.a)
    endif()
  endif()
  MESSAGE(STATUS "Boost unit test framework: ${BOOST_UNIT_TEST_FRAMEWORK_LIB_STATIC}")

  #see http://www.boost.org/doc/libs/1_57_0/libs/test/doc/html/utf/compilation.html#utf.flag.alt-init-api
  set_target_properties(boost_unit_test_framework PROPERTIES IMPORTED_LOCATION ${BOOST_UNIT_TEST_FRAMEWORK_LIB_STATIC})
endif()


MESSAGE(STATUS "unit testing: SDL_UNIT_TEST_MAIN ${SDL_UNIT_TEST_MAIN} ... SDL_TRY_UNIT_TEST_DYN_LINK ${SDL_TRY_UNIT_TEST_DYN_LINK}")
sdl_quiet_compiled_setting(SDL_UNIT_TEST_MAIN ${SDL_UNIT_TEST_MAIN})

MESSAGE(STATUS "Boost Include Directories: ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "Boost Library Directories: ${Boost_LIBRARY_DIRS}")

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost libraries not found")
endif()

sdl_rpath(${Boost_LIBRARY_DIRS})

find_package(YamlCpp)
message(STATUS "yaml-cpp library: ${YAML_CPP_LIBRARY}")
message(STATUS "yaml-cpp include directory: ${YAML_CPP_INCLUDE_DIR}")

find_package(ZLIB)
MESSAGE(STATUS "ZLib Found: ${ZLIB_LIBRARIES}")
sdl_quiet_compiled_setting(HAVE_ZLIB 1)


if(NOT ANDROID)
  find_package (Log4CXX)
  MESSAGE(STATUS "log4cxx Libraries: ${LOG4CXX_LIBRARIES}")
  MESSAGE(STATUS "log4cxx Include Directories: ${LOG4CXX_INCLUDE_DIR}")
endif()


# L-BFGS
# Would use float (i.e., value 32) instead of double but gets aborts
# due to rounding errors:
set(LBFGS_FLOAT "64" CACHE STRING "Float type for L-BFGS")
sdl_compiled_setting(LBFGS_FLOAT ${LBFGS_FLOAT})
if(NOT DEFINED LBFGS_ROOT)
  if(${LBFGS_FLOAT} MATCHES 32)
    set(LBFGS_ROOT "${SDL_EXTERNALS_PATH}/libraries/liblbfgs-b2a54a9-float") # L-BFGS compiled with float
  else()
    set(LBFGS_ROOT "${SDL_EXTERNALS_PATH}/libraries/liblbfgs-1.10") # L-BFGS compiled with double
  endif()
endif()
find_package(Lbfgs)
MESSAGE(STATUS "libLBFGS Libraries: ${LBFGS_LIB}")
MESSAGE(STATUS "libLBFGS Include Directories: ${LBFGS_INCLUDE_DIR}")

if(WIN32)
  # Windows linker problem: OpenFst constant kNoSymbol is defined in
  # multiple Hypergraph objects
  set(HAVE_OPENFST 0)
endif()

if(HAVE_OPENFST)
  set(OPENFST_ROOT "${SDL_SHARED_EXTERNALS_PATH}/openfst-1.2.10")
  find_package(OpenFst)
  if(OPENFST_FOUND)
    message(STATUS "OpenFst include directory: ${OPENFST_INCLUDE_DIR}")
    sdl_compiled_setting(HAVE_OPENFST 1)
  else()
    set(HAVE_OPENFST 0)
  endif()
endif()

find_package(UTF8)

# By default, create only the dynamic XMT lib. The user can override this
# behaviour using the CMAKE variable SDL_LIB_TYPE which supports the values:
# - "Shared" -- (Default mode) xmtShell and related binaries will link dynamically to this
# - "Static" -- xmtShell and related binaries will link statically to this
# - "All" -- builds both libraries and links to the xmtShell and related binaries statically
if(NOT DEFINED SDL_LIB_TYPE OR SDL_LIB_TYPE STREQUAL "Shared")
  set(SDL_LIB "xmt_shared")
else()
  set(SDL_LIB "sdl_static")
endif()

if(BUILD_KENLM)
  add_subdirectory(${SDL_EXTERNALS_SOURCE_PATH}/KenLM ${CMAKE_CURRENT_BINARY_DIR}/KenLM)
endif()

list(APPEND subDirectories
  Util
  Vocabulary
  Hypergraph
  Optimization
  Config
  HypDemo
  CrfDemo
  )


#CTest and CDash
if(NOT ANDROID)
  INCLUDE(CTest)

  # for clang static analyzer
  set(CTEST_CUSTOM_WARNING_EXCEPTION
".*warning: 'type' overrides a member function but is not marked 'override' [-Winconsistent-missing-override].*"
".*small_vector.hpp.*warning: Call to 'malloc' has an allocation size of 0 bytes.*"
".*stl_uninitialized.h:.*warning: Forming reference to null pointer.*"
".*stl_vector.h:.* warning: Returning null reference.*"
"warning: Value stored to 'yymsg' is never read"
"warning: Value stored to 'yytoken' is never read"
)

endif()


set(UTIL_LIBRARY_NAME sdl_util)

set(INDIRECT_Vocabulary_LIB_DEPENDENCIES
  ${TBB_LIBRARY}
  ${YAML_CPP_LIBRARY}
  ${APR_LIBRARY}
  ${APR_UTIL_LIBRARY}
  ${LOG4CXX_LIBRARIES}
  ${ICU_LIBRARY}
  ${ZLIB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${GCC5_LIBRARIES}
  )


if (LINUX)
  list(APPEND INDIRECT_Vocabulary_LIB_DEPENDENCIES rt)
endif()

set(INDIRECT_UTIL_LIB_DEPENDENCIES
  ${INDIRECT_Vocabulary_LIB_DEPENDENCIES}
  Vocabulary
  ${BZIP2_LIBRARIES}
  )

if (WIN32)
  list(APPEND INDIRECT_UTIL_LIB_DEPENDENCIES psapi.lib)
endif()

if(ZLIB_FOUND)
  # kenlm was built with this
  sdl_quiet_compiled_setting(HAVE_LIBZ 1)
else()
  message(FATAL_ERROR "not found: libz")
endif()

# Util and prereqs - used by most projects (except a few that might
# use Util header-only)
set(UTIL_LIBRARIES ${UTIL_LIBRARY_NAME} ${INDIRECT_UTIL_LIB_DEPENDENCIES})

macro(sdl_util_includes)
  include_directories(${Boost_INCLUDE_DIRS})
  include_directories(${CMAKE_SOURCE_DIR}/..)
  include_directories(${LOG4CXX_INCLUDE_DIR})
  include_directories(${ICU_INCLUDE_DIR})
  include_directories(${YAML_CPP_INCLUDE_DIR})
  include_directories(${TBB_INCLUDE_DIR})
  include_directories(${UTF8_INCLUDE_DIR})
  include_directories(${ZEROMQ_INCLUDE_DIR})
  include_directories(${ZLIB_INCLUDE_DIR})
endmacro()


set(INDIRECT_CONFIG_LIB_DEPENDENCIES
  ${UTIL_LIBRARIES}
  )

# most projects can just depend on Config and ${UTIL_LIBRARIES} because
# YAML_CPP_LIBRARY is hidden inside Config/src/*.cpp. some projects w/ configure
# don't need Config at all because they have only a template<...> configure(...)

#//TODO: in some cases only the Main executable (and not the library) needs
#YAML_CPP_LIBRARY. in other cases we can remove the yaml-node constructors and
#require caller use the configure library for that purpose
set(CONFIG_LIBRARIES Config ${INDIRECT_CONFIG_LIB_DEPENDENCIES})

sdl_finish_cflags()

# Add subdirectories and find their unit test executables.

if(OPEN_SOURCE_RELEASE)
  MESSAGE(STATUS "OPEN_SOURCE_RELEASE: setting defines")
  sdl_compiled_setting(HYP_OPEN_SOURCE_RELEASE 1) #necessary
  sdl_compiled_setting(SDL_HYPERGRAPH_FS_ANNOTATIONS 0) # better performance (but you may be interested in using them - see code)

  sdl_compiled_setting(SDL_ASSERT_THREAD_SPECIFIC 0) #necessary
  sdl_compiled_setting(SDL_OBJECT_COUNT 0) #necessary
  sdl_compiled_setting(SDL_ENCRYPT 0) #necessary?

  if(APPLE)
    sdl_quiet_compiled_setting(SDL_FIX_LOCALE 0) # optional except on my mac
    sdl_quiet_compiled_setting(SDL_BOOST_LOCALE 0) # optional
  endif()
  #  sdl_compiled_setting(SDL_FAST_COUT 0) # optional
  #  sdl_compiled_setting(GRAEHL_FILEARGS_DEFAULT_LARGE_BUF 0) #optional
else()
  sdl_quiet_compiled_setting(SDL_HYPERGRAPH_FS_ANNOTATIONS 1)
  if(SDL_DEVELOPMENT)
    MESSAGE(STATUS "!OPEN_SOURCE_RELEASE AND SDL_DEVELOPMENT: SDL_ASSERT_THREAD_SPECIFIC")
    # if not set, then enabled for debug only:
    sdl_quiet_compiled_setting(SDL_ASSERT_THREAD_SPECIFIC 1)
  endif()
  if(SDL_DEBUG)
    if(SDL_PRODUCTION)
      sdl_quiet_compiled_setting(SDL_OBJECT_COUNT 0)
    else()
      sdl_quiet_compiled_setting(SDL_OBJECT_COUNT 0)
      # ~1% runtime cost but can tell you exactly what type of object leaks in single thread
    endif()
  else()
    sdl_quiet_compiled_setting(SDL_OBJECT_COUNT 0)
  endif()
endif()

# IMPORTANT: if you want to make changes to settings that affect these, do it
# before this point in the file
foreach(subDirectory ${subDirectories})
  add_subdirectory(${subDirectory})
  if(NOT ANDROID)
    set(testFiles "")
    aux_source_directory("${subDirectory}/test" testFiles)
    foreach(testFile ${testFiles})
      get_filename_component(testFile ${testFile} NAME)
      string(REGEX REPLACE ".cpp$" "" testFile ${testFile})
      list(APPEND testExecutables "${testFile}")
    endforeach()
  endif()
endforeach()
message(STATUS "-- Test Executables: ${testExecutables}")

#configure a header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/CMakeConfig.in"
  "${PROJECT_SOURCE_DIR}/CMakeConfig.hpp")

MESSAGE(STATUS "Unit test (check only) executables: ${testExecutables}")

# Add custom target "check" that builds unit test executables and runs
# unit tests:
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose
  DEPENDS ${testExecutables})

#
# Generate Doxygen target
#
add_custom_target(doc ${CMAKE_SOURCE_DIR}/doxygenate WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
