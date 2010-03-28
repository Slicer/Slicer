# Generate the GenerateCLPConfig.cmake file in the build tree and configure 
# one the installation tree.

# Following the standard pattern, UseGenerateCLP.cmake is a configured file
#
if(GenerateCLP_EXE_PATH)
  set(GENERATECLP_EXE "${GenerateCLP_EXE_PATH}")
else(GenerateCLP_EXE_PATH)
  if(Slicer3_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${Slicer3_BINARY_DIR}/bin)
  else(Slicer3_BINARY_DIR)
    set(USE_CONFIG_FILE_INSTALL_DIR ${GenerateCLP_BINARY_DIR})
  endif(Slicer3_BINARY_DIR)
  set(GENERATECLP_EXE "${USE_CONFIG_FILE_INSTALL_DIR}/${CMAKE_CFG_INTDIR}/GenerateCLP")
endif(GenerateCLP_EXE_PATH)

# Settings specific to build trees
#
#
set(GenerateCLP_USE_FILE_CONFIG ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake)
configure_file(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
  ${GenerateCLP_USE_FILE_CONFIG}
  @ONLY IMMEDIATE)

set(GenerateCLP_INCLUDE_DIRS_CONFIG ${GenerateCLP_BINARY_DIR} ${GenerateCLP_SOURCE_DIR})
set(GenerateCLP_LIBRARY_DIRS_CONFIG ${GenerateCLP_BINARY_DIR})
set(GENERATECLP_EXE_CONFIG "${GENERATECLP_EXE}")
set(ITK_DIR_CONFIG ${ITK_DIR})
configure_file(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
  ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake @ONLY IMMEDIATE)


# Settings specific for installation trees
#
#
configure_file(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
  ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
  @ONLY IMMEDIATE)

configure_file(${GenerateCLP_SOURCE_DIR}/GenerateCLPInstallConfig.cmake.in
  ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install @ONLY IMMEDIATE)
