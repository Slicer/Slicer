
set(proj tbb)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported!")
endif()

set(tbb_ver "2018_20171205oss")
if (WIN32)
  set(tbb_file "tbb${tbb_ver}_win.zip")
  set(tbb_md5 "e37abf02d74a638f7a6629c992f23918")
elseif (APPLE)
  set(tbb_file "tbb${tbb_ver}_mac.tgz")
  set(tbb_md5 "ff7a02f58fee4e2e637db6da19a21806")
else ()
  set(tbb_file "tbb${tbb_ver}_lin.tgz")
  set(tbb_md5 "d637d29f59ee31fe5830a0366e2e973a")
endif ()

#------------------------------------------------------------------------------
set(TBB_INSTALL_DIR "${CMAKE_BINARY_DIR}/${proj}-install")
ExternalProject_Message(${proj} "TBB_INSTALL_DIR:${TBB_INSTALL_DIR}")

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  URL https://github.com/Slicer/SlicerBinaryDependencies/releases/download/tbb/${tbb_file}
  URL_MD5 ${tbb_md5}
  DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
  SOURCE_DIR ${TBB_INSTALL_DIR}
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  )

if(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
  set(tbb_archdir intel64)
else()
  set(tbb_archdir ia32)
endif()

if (WIN32)
  if (NOT MSVC_VERSION VERSION_GREATER 1500)
    message(FATAL_ERROR "At least Visual Studio 10.0 is required")
  elseif (NOT MSVC_VERSION VERSION_GREATER 1600)
    set(tbb_vsdir vc10)
  elseif (NOT MSVC_VERSION VERSION_GREATER 1700)
    set(tbb_vsdir vc11)
  elseif (NOT MSVC_VERSION VERSION_GREATER 1800)
    set(tbb_vsdir vc12)
  elseif (NOT MSVC_VERSION VERSION_GREATER 1900)
    set(tbb_vsdir vc14)
  elseif (NOT MSVC_VERSION VERSION_GREATER 1919)
    # VS2017 is expected to be binary compatible with VS2015
    set(tbb_vsdir vc14)
  elseif (NOT MSVC_VERSION VERSION_GREATER 1929)
    # VS2019 is expected to be binary compatible with VS2015
    set(tbb_vsdir vc14)
  elseif (tbb_enabled)
    message(FATAL_ERROR "TBB does not support your Visual Studio compiler. [MSVC_VERSION: ${MSVC_VERSION}]")
  endif ()
  set(tbb_libdir lib/${tbb_archdir}/${tbb_vsdir})
  set(tbb_bindir bin/${tbb_archdir}/${tbb_vsdir})
elseif (APPLE)
  set(tbb_libdir "lib")
  set(tbb_bindir "bin")
else ()
  set(tbb_libdir "lib/${tbb_archdir}/gcc4.4")
  set(tbb_bindir "bin")
  set(tbb_libsuffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif ()

if (NOT WIN32)
  set(tbb_lib_prefix "lib")
else ()
  set(tbb_lib_prefix "")
endif ()

if (NOT tbb_libsuffix)
  set(tbb_libsuffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
  if (WIN32)
    set(tbb_libsuffix ${CMAKE_IMPORT_LIBRARY_SUFFIX})
  endif ()
endif ()

# TODO: apply this patch
# if (UNIX AND NOT APPLE)
  # superbuild_apply_patch(tbb gcc5x-warning-fix
    # "Tell TBB about GCC 5.1 stdlib support")
# endif()

#------------------------------------------------------------------------------

set(TBB_INCLUDE_DIR "${TBB_INSTALL_DIR}/include")
set(TBB_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbb_debug${tbb_libsuffix}")
set(TBB_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbb${tbb_libsuffix}")

set(TBB_MALLOC_INCLUDE_DIR "${TBB_INSTALL_DIR}/include/tbb")
set(TBB_MALLOC_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbbmalloc_debug${tbb_libsuffix}")
set(TBB_MALLOC_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbbmalloc${tbb_libsuffix}")

set(TBB_MALLOC_PROXY_INCLUDE_DIR "${TBB_INSTALL_DIR}/include/tbb")
set(TBB_MALLOC_PROXY_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbbmalloc_proxy_debug${tbb_libsuffix}")
set(TBB_MALLOC_PROXY_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/${tbb_lib_prefix}tbbmalloc_proxy${tbb_libsuffix}")

set(TBB_BIN_DIR "${TBB_INSTALL_DIR}/${tbb_bindir}")
set(TBB_LIB_DIR "${TBB_INSTALL_DIR}/${tbb_libdir}")
mark_as_superbuild(
  VARS
    TBB_BIN_DIR:PATH
    TBB_LIB_DIR:PATH
  )

#-----------------------------------------------------------------------------

ExternalProject_GenerateProjectDescription_Step(${proj}
  VERSION ${tbb_ver}
  LICENSE_FILES "https://raw.githubusercontent.com/01org/tbb/tbb_2018/LICENSE"
  )

#-----------------------------------------------------------------------------
# Launcher setting specific to build tree

set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD "${TBB_INSTALL_DIR}/${tbb_bindir}")
mark_as_superbuild(
  VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
  LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
  )
