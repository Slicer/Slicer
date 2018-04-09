
set(proj tbb)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported!")
endif()

set(tbb_ver "2018_20171205oss")
if (WIN32)
  set(tbb_file "tbb${tbb_ver}_win.zip")
  #set(tbb_md5 "e7bbf293cdb5a50ca81347c80168956d")
elseif (APPLE)
  set(tbb_file "tbb${tbb_ver}_osx.tgz")
  #set(tbb_md5 "a767d7a8b375e6b054e44e2317d806b8")
else ()
  set(tbb_file "tbb${tbb_ver}_lin_0.tgz")
  #set(tbb_md5 "ab5df80a65adf423b14637a1f35814b2")
endif ()

#------------------------------------------------------------------------------
set(TBB_INSTALL_DIR "${CMAKE_BINARY_DIR}/${proj}-install")
ExternalProject_Message(${proj} "TBB_INSTALL_DIR:${TBB_INSTALL_DIR}")

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  URL https://www.paraview.org/files/dependencies/${tbb_file}
  #URL_MD5 ${tbb_md5}
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
  elseif (tbb_enabled)
    message(FATAL_ERROR "TBB does not support your Visual Studio compiler.")
  endif ()
  set(tbb_libdir lib/${tbb_archdir}/${tbb_vsdir})
  set(tbb_bindir bin/${tbb_archdir}/${tbb_vsdir})
elseif (APPLE)
  set(tbb_libdir "lib")
  set(tbb_bindir "bin")
else ()
  set(tbb_libdir "lib/${tbb_archdir}/gcc4.4")
  set(tbb_bindir "bin/${tbb_archdir}/gcc4.4")
  set(tbb_libsuffix "${CMAKE_SHARED_LIBRARY_SUFFIX}*")
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
set(TBB_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/tbb_debug${tbb_libsuffix}")
set(TBB_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/tbb${tbb_libsuffix}")

set(TBB_MALLOC_INCLUDE_DIR "${TBB_INSTALL_DIR}/include/tbb")
set(TBB_MALLOC_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/tbbmalloc_debug${tbb_libsuffix}")
set(TBB_MALLOC_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/tbbmalloc${tbb_libsuffix}")

set(TBB_MALLOC_PROXY_INCLUDE_DIR "${TBB_INSTALL_DIR}/include/tbb")
set(TBB_MALLOC_PROXY_LIBRARY_DEBUG "${TBB_INSTALL_DIR}/${tbb_libdir}/tbbmalloc_proxy_debug${tbb_libsuffix}")
set(TBB_MALLOC_PROXY_LIBRARY_RELEASE "${TBB_INSTALL_DIR}/${tbb_libdir}/tbbmalloc_proxy${tbb_libsuffix}")

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
