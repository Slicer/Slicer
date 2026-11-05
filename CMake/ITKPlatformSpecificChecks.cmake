include(itkCheckCXXAcceptsFlags)

# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings
if(WIN32 AND NOT MINGW)
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

if(WIN32)
  # Some libraries (e.g. vxl libs) have no dllexport markup, so we can
  # build full shared libraries only with the GNU toolchain. For non
  # gnu compilers on windows, only Common is shared.  This allows for
  # plugin type applications to use a dll for ITKCommon which will contain
  # the static for Modified time.
  if(CMAKE_COMPILER_IS_GNUCXX)
    # Export all symbols explicitly for a native Windows build with the MinGW tools.
    if(MINGW)
      set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
        "-shared -Wl,--export-all-symbols -Wl,--enable-auto-import")
      set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
        "-shared -Wl,--export-all-symbols -Wl,--enable-auto-import")
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

# mingw thread support
if(MINGW)
  set(ITK_REQUIRED_CXX_FLAGS "${ITK_REQUIRED_CXX_FLAGS} -mthreads")
  set(ITK_REQUIRED_C_FLAGS "${ITK_REQUIRED_C_FLAGS} -mthreads")
  set(ITK_REQUIRED_LINK_FLAGS "${ITK_REQUIRED_LINK_FLAGS} -mthreads")
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
      message(FATAL_ERROR "-ffloat-store causes runtime bugs on gcc 3.2.3 (apparently not on gcc 3.4.3, but the exact criteria is not known).  It must be removed from your compilation options.")
    endif()
  endforeach()
endif()

#---------------------------------------------------------------
# run try compiles and tests for ITK
#include(CMake/itkTestFriendTemplatedFunction.cmake)
