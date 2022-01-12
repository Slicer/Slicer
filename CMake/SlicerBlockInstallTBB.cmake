# -------------------------------------------------------------------------
# Find and install TBB Libs
# -------------------------------------------------------------------------

set(TBB_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")

if(NOT EXISTS "${TBB_BIN_DIR}")
  message(FATAL_ERROR "CMake variable TBB_BIN_DIR is set to a nonexistent directory: ${TBB_BIN_DIR}")
endif()
if(NOT EXISTS "${TBB_LIB_DIR}")
  message(FATAL_ERROR "CMake variable TBB_LIB_DIR is set to a nonexistent directory: ${TBB_LIB_DIR}")
endif()

if(WIN32)
  install(
    FILES
      ${TBB_BIN_DIR}/tbb.dll
      ${TBB_BIN_DIR}/tbbmalloc.dll
      ${TBB_BIN_DIR}/tbbmalloc_proxy.dll
    CONFIGURATIONS Release
    DESTINATION bin COMPONENT Runtime)
  install(
    FILES
      ${TBB_BIN_DIR}/tbb_debug.dll
      ${TBB_BIN_DIR}/tbbmalloc_debug.dll
      ${TBB_BIN_DIR}/tbbmalloc_proxy_debug.dll
    CONFIGURATIONS Debug
    DESTINATION bin COMPONENT Runtime)
elseif(APPLE)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb.dylib
      ${TBB_LIB_DIR}/libtbbmalloc.dylib
      ${TBB_LIB_DIR}/libtbbmalloc_proxy.dylib
    CONFIGURATIONS Release
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb_debug.dylib
      ${TBB_LIB_DIR}/libtbbmalloc_debug.dylib
      ${TBB_LIB_DIR}/libtbbmalloc_proxy_debug.dylib
    CONFIGURATIONS Debug
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
elseif(UNIX)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb.so.2
      ${TBB_LIB_DIR}/libtbbmalloc.so.2
      ${TBB_LIB_DIR}/libtbbmalloc_proxy.so.2
    CONFIGURATIONS Release
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb_debug.so.2
      ${TBB_LIB_DIR}/libtbbmalloc_debug.so.2
      ${TBB_LIB_DIR}/libtbbmalloc_proxy_debug.so.2
    CONFIGURATIONS Debug
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
  if (CMAKE_BUILD_TYPE EQUAL "Debug")
    slicerStripInstalledLibrary(
      FILES
        "${TBB_INSTALL_LIB_DIR}/libtbb_debug.so.2"
        "${TBB_INSTALL_LIB_DIR}/libtbbmalloc_debug.so.2"
        "${TBB_INSTALL_LIB_DIR}/libtbbmalloc_proxy_debug.so.2"
      COMPONENT Runtime)
  else ()
    slicerStripInstalledLibrary(
      FILES
        "${TBB_INSTALL_LIB_DIR}/libtbb.so.2"
        "${TBB_INSTALL_LIB_DIR}/libtbbmalloc.so.2"
        "${TBB_INSTALL_LIB_DIR}/libtbbmalloc_proxy.so.2"
      COMPONENT Runtime)
  endif()
endif()