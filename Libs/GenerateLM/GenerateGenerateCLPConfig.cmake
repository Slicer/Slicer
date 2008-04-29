# Generate the GenerateCLPConfig.cmake file in the build tree and configure 
# one the installation tree.

# Settings specific to build trees
#
#
SET(GenerateCLP_INCLUDE_DIRS_CONFIG ${GenerateCLP_BINARY_DIR} ${GenerateCLP_SOURCE_DIR})
SET(GenerateCLP_LIBRARY_DIRS_CONFIG ${GenerateCLP_BINARY_DIR})
SET(GenerateCLP_USE_FILE_CONFIG ${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)
SET(GENERATECLP_EXE_CONFIG ${GENERATECLP_EXE})
SET(TCLAP_DIR_CONFIG ${TCLAP_DIR})
SET(ModuleDescriptionParser_DIR_CONFIG ${ModuleDescriptionParser_DIR})
SET(ITK_DIR_CONFIG ${ITK_DIR})


# Configure GenerateCLPConfig.cmake for the install tree.
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
               ${GenerateCLP_BINARY_DIR}/GenerateCLPConfig.cmake @ONLY IMMEDIATE)



# Settings specific for installation trees
#
#
SET(GenerateCLP_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/include/GenerateCLP)
SET(GenerateCLP_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateCLP)
SET(GenerateCLP_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateCLP/UseGenerateCLP.cmake)
SET(GENERATECLP_EXE_CONFIG ${CMAKE_INSTALL_PREFIX}/bin/GenerateCLP)
SET(TCLAP_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/ModuleDescriptionParser)
SET(ITK_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/InsightToolkit)



# Configure GenerateCLPConfig.cmake for the install tree.
CONFIGURE_FILE(${GenerateCLP_SOURCE_DIR}/GenerateCLPConfig.cmake.in
               ${GenerateCLP_BINARY_DIR}/install/GenerateCLPConfig.cmake @ONLY IMMEDIATE)
