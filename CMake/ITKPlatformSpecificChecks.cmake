include(itkCheckCXXAcceptsFlags)

# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings
if(WIN32)
   if(NOT CYGWIN)
     if(NOT MINGW)
       if(NOT ITK_ENABLE_VISUAL_STUDIO_DEPRECATED_C_WARNINGS)
         add_definitions(
           -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
           -D_CRT_IS_WCTYPE_NO_DEPRECATE
           -D_CRT_MANAGED_FP_NO_DEPRECATE
           -D_CRT_NONSTDC_NO_DEPRECATE
           -D_CRT_SECURE_NO_DEPRECATE
           -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
           -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
           -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
           -D_CRT_VCCLRIT_NO_DEPRECATE
           -D_SCL_SECURE_NO_DEPRECATE
           )
       endif()
     endif()
   endif()
endif()

if(WIN32)
  # Some libraries (e.g. vxl libs) have no dllexport markup, so we can
  # build full shared libraries only with the GNU toolchain. For non
  # gnu compilers on windows, only Common is shared.  This allows for
  # plugin type applications to use a dll for ITKCommon which will contain
  # the static for Modified time.
  if(CMAKE_COMPILER_IS_GNUCXX)
    # CMake adds --enable-all-exports on Cygwin (since Cygwin is
    # supposed to be UNIX-like), but we need to add it explicitly for
    # a native windows build with the MinGW tools.
    if(MINGW)
      set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
        "-shared -Wl,--export-all-symbols -Wl,--enable-auto-import")
      set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
        "-shared -Wl,--export-all-symbols -Wl,--enable-auto-import")
      set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-import")
    endif()
    if(CYGWIN)
      set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-auto-import")
    endif()
  #else()
   #if(BUILD_SHARED_LIBS)
   #  set(ITK_LIBRARY_BUILD_TYPE "SHARED")
   #else()
   #  set(ITK_LIBRARY_BUILD_TYPE "STATIC")
   #endif()
   #set(BUILD_SHARED_LIBS OFF)
  endif()
endif()

#-----------------------------------------------------------------------------
#ITK requires special compiler flags on some platforms.
if(CMAKE_COMPILER_IS_GNUCXX)
 set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -Wall -Wno-uninitialized -Wno-unused-parameter")
 set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -Wall")
 itkCHECK_CXX_ACCEPTS_FLAGS("-Wno-deprecated" CXX_HAS_DEPRECATED_FLAG)
 if(CXX_HAS_DEPRECATED_FLAG)
   set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -Wno-deprecated")
 endif()
 if(APPLE)
   # -no-cpp-precomp and -Wno-long-double were compiler flags present
   # only in Apple's gcc and not in the FSF gcc. The flags are obsolete
   # and totally removed in gcc 4.2 and later. I believe they are only
   # needed with gcc 3.3 and earlier.
   execute_process(COMMAND "${CMAKE_C_COMPILER}" --version
     OUTPUT_VARIABLE _version ERROR_VARIABLE _version)
   if("${_version}" MATCHES "gcc.*3\\.3.*Apple")
     set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -no-cpp-precomp")
     set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -no-cpp-precomp")
   endif()
   itkCHECK_CXX_ACCEPTS_FLAGS("-Wno-long-double" CXX_HAS_LONGDOUBLE_FLAG)
   if(CXX_HAS_LONGDOUBLE_FLAG)
     set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -Wno-long-double")
     set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -Wno-long-double")
   endif()

   option(ITK_USE_64BITS_APPLE_TRUNCATION_WARNING "Turn on warnings on 64bits to 32bits truncations." OFF)
   mark_as_advanced(ITK_USE_64BITS_APPLE_TRUNCATION_WARNING)
 endif()

 # gcc must have -msse2 option to enable sse2 support
 if(VNL_CONFIG_ENABLE_SSE2 OR VNL_CONFIG_ENABLE_SSE2_ROUNDING)
   set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -msse2")
 endif()
endif()

#-----------------------------------------------------------------------------

# for the gnu compiler a -D_PTHREADS is needed on sun
# for the native compiler a -mt flag is needed on the sun
if(CMAKE_SYSTEM MATCHES "SunOS.*")
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -D_PTHREADS")
    set(ITK_REQUIRED_LINK_FLAGS "${ITK_REQUIRED_LINK_FLAGS} -lrt")
  else()
    set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -mt")
    set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -mt")
  endif()
endif()

# mingw thread support
if(MINGW)
  set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -mthreads")
  set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -mthreads")
  set(ITK_REQUIRED_LINK_FLAGS "${ITK_REQUIRED_LINK_FLAGS} -mthreads")
endif()

# Add flags for the SUN compiler to provide all the methods for std::allocator.
#
itkCHECK_CXX_ACCEPTS_FLAGS("-features=no%anachronisms" SUN_COMPILER)
if(SUN_COMPILER)
  itkCHECK_CXX_ACCEPTS_FLAGS("-library=stlport4" SUN_COMPILER_HAS_STL_PORT_4)
  if(SUN_COMPILER_HAS_STL_PORT_4)
    set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -library=stlport4")
  endif()
endif()

#-----------------------------------------------------------------------------
# The frename-registers option does not work due to a bug in the gnu compiler.
# It must be removed or data errors will be produced and incorrect results
# will be produced.  This is first documented in the gcc4 man page.
if(CMAKE_COMPILER_IS_GNUCXX)
  set(ALL_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_MODULE_LINKER_FLAGS}" )
  separate_arguments(ALL_FLAGS)
  foreach(COMP_OPTION ${ALL_FLAGS})
    if("${COMP_OPTION}" STREQUAL "-frename-registers")
      message(FATAL_ERROR "-frename-registers causes runtime bugs.  It must be removed from your compilation options.")
    endif()
    if("${COMP_OPTION}" STREQUAL "-ffloat-store")
      message(FATAL_ERROR "-ffloat-store causes runtime bugs on gcc 3.2.3 (appearently not on gcc 3.4.3, but the exact criteria is not known).  It must be removed from your compilation options.")
    endif()
  endforeach()
endif()

#---------------------------------------------------------------
# run try compiles and tests for ITK
#include(CMake/itkTestFriendTemplatedFunction.cmake)
