# Generate the GenerateCLPConfig.cmake file in the build tree and configure 
# one the installation tree.

# Following the standard pattern, UseGenerateCLP.cmake is a configured file
#
IF(Slicer3_BINARY_DIR)
  SET(USE_CONFIG_FILE_INSTALL_DIR ${Slicer3_BINARY_DIR}/bin)
ELSE(Slicer3_BINARY_DIR)
  SET(USE_CONFIG_FILE_INSTALL_DIR ${GenerateCLP_BINARY_DIR})
ENDIF(Slicer3_BINARY_DIR)
SET(GENERATECLP_EXE ${USE_CONFIG_FILE_INSTALL_DIR}/${CMAKE_CFG_INTDIR}/GenerateCLP)

# Settings specific to build trees
#
#
SET(GenerateCLP_USE_FILE_CONFIG ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake)
SET(TCLAP_DIR_CONFIG ${TCLAP_DIR})
SET(ModuleDescriptionParser_DIR_CONFIG ${ModuleDescriptionParser_DIR})
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
SET(GenerateCLP_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateCLP/UseGenerateCLP.cmake)
SET(TCLAP_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/ModuleDescriptionParser)
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake.in
               ${GenerateCLP_BINARY_DIR}/UseGenerateCLP.cmake_install
               @ONLY IMMEDIATE)

SET(GenerateCLP_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/include/GenerateCLP)
SET(GenerateCLP_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateCLP)
SET(GENERATECLP_EXE_CONFIG ${CMAKE_INSTALL_PREFIX}/bin/GenerateCLP)
SET(ITK_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/InsightToolkit)
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
               ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake_install @ONLY IMMEDIATE)
