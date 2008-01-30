# Generate the GenerateCLPConfig.cmake file in the build tree and configure 
# one the installation tree.

# Following the standard pattern, UseGenerateCLP.cmake is a configured file
#
IF(GenerateCLP_EXE_PATH)
  SET(GENERATECLP_EXE ${GenerateCLP_EXE_PATH})
ELSE(GenerateCLP_EXE_PATH)
  IF(Slicer3_BINARY_DIR)
    SET(USE_CONFIG_FILE_INSTALL_DIR ${Slicer3_BINARY_DIR}/bin)
  ELSE(Slicer3_BINARY_DIR)
    SET(USE_CONFIG_FILE_INSTALL_DIR ${GenerateCLP_BINARY_DIR})
  ENDIF(Slicer3_BINARY_DIR)
  SET(GENERATECLP_EXE ${USE_CONFIG_FILE_INSTALL_DIR}/${CMAKE_CFG_INTDIR}/GenerateCLP)
ENDIF(GenerateCLP_EXE_PATH)

# Settings specific to build trees
#
#
SET(GenerateCLP_USE_FILE_CONFIG ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake)
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
               ${GenerateCLP_USE_FILE_CONFIG}
               @ONLY IMMEDIATE)

SET(GenerateCLP_INCLUDE_DIRS_CONFIG ${GenerateCLP_BINARY_DIR} ${GenerateCLP_SOURCE_DIR})
SET(GenerateCLP_LIBRARY_DIRS_CONFIG ${GenerateCLP_BINARY_DIR})
SET(GENERATECLP_EXE_CONFIG ${GENERATECLP_EXE})
SET(ITK_DIR_CONFIG ${ITK_DIR})
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
               ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake @ONLY IMMEDIATE)


# Settings specific for installation trees
#
#
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
               ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
               @ONLY IMMEDIATE)

CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/GenerateCLPInstallConfig.cmake.in
               ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install @ONLY IMMEDIATE)
