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
    DESTINATION bin COMPONENT Runtime)
elseif(APPLE)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb.dylib
      ${TBB_LIB_DIR}/libtbbmalloc.dylib
      ${TBB_LIB_DIR}/libtbbmalloc_proxy.dylib
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
elseif(UNIX)
  install(
    FILES
      ${TBB_LIB_DIR}/libtbb.so.2
      ${TBB_LIB_DIR}/libtbbmalloc.so.2
      ${TBB_LIB_DIR}/libtbbmalloc_proxy.so.2
    DESTINATION ${TBB_INSTALL_LIB_DIR} COMPONENT Runtime)
  slicerStripInstalledLibrary(
    FILES
      "${TBB_INSTALL_LIB_DIR}/libtbb.so.2"
      "${TBB_INSTALL_LIB_DIR}/libtbbmalloc.so.2"
      "${TBB_INSTALL_LIB_DIR}/libtbbmalloc_proxy.so.2"
    COMPONENT Runtime)
endif()
