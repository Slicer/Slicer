# Generate the GenerateLMConfig.cmake file in the build tree and configure 
# one the installation tree.

# Following the standard pattern, UseGenerateLM.cmake is a configured file
#
if(GenerateLM_EXE_PATH)
  set(GENERATELM_EXE ${GenerateLM_EXE_PATH})
else(GenerateLM_EXE_PATH)
  if(Slicer3_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${Slicer3_BINARY_DIR}/bin)
  else(Slicer3_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${GenerateLM_BINARY_DIR})
  endif(Slicer3_BINARY_DIR)
  set(GENERATELM_EXE ${USE_CONFIG_FILE_INSTALL_DIR}/${CMAKE_CFG_INTDIR}/GenerateLM)
endif(GenerateLM_EXE_PATH)

# Settings specific to build trees
#
#
set(GenerateLM_USE_FILE_CONFIG ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake)
configure_file(${GenerateLM_SOURCE_DIR}/UseGenerateLM.cmake.in
  ${GenerateLM_USE_FILE_CONFIG}
  @ONLY IMMEDIATE)

set(GenerateLM_INCLUDE_DIRS_CONFIG ${GenerateLM_BINARY_DIR} ${GenerateLM_SOURCE_DIR})
set(GenerateLM_LIBRARY_DIRS_CONFIG ${GenerateLM_BINARY_DIR})
set(GENERATELM_EXE_CONFIG ${GENERATELM_EXE})
set(ITK_DIR_CONFIG ${ITK_DIR})
configure_file(${GenerateLM_SOURCE_DIR}/GenerateLMConfig.cmake.in
  ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake @ONLY IMMEDIATE)

# Settings specific for installation trees
#
#
configure_file(${GenerateLM_SOURCE_DIR}/UseGenerateLM.cmake.in
  ${GenerateLM_BINARY_DIR}/UseGenerateLM.cmake_install
  @ONLY IMMEDIATE)

configure_file(${GenerateLM_SOURCE_DIR}/GenerateLMInstallConfig.cmake.in
  ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake_install @ONLY IMMEDIATE)
