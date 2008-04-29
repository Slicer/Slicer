# Generate the GenerateLMConfig.cmake file in the build tree and configure 
# one the installation tree.

# Settings specific to build trees
#
#
SET(GenerateLM_INCLUDE_DIRS_CONFIG ${GenerateLM_BINARY_DIR} ${GenerateLM_SOURCE_DIR})
SET(GenerateLM_LIBRARY_DIRS_CONFIG ${GenerateLM_BINARY_DIR})
SET(GenerateLM_USE_FILE_CONFIG ${GenerateLM_SOURCE_DIR}/UseGenerateLM.cmake)
SET(GENERATELM_EXE_CONFIG ${GENERATELM_EXE})
SET(TCLAP_DIR_CONFIG ${TCLAP_DIR})
SET(LoadableModule_DIR_CONFIG ${LoadableModule_DIR})
SET(ITK_DIR_CONFIG ${ITK_DIR})


# Configure GenerateLMConfig.cmake for the install tree.
CONFIGURE_FILE(${GenerateLM_SOURCE_DIR}/GenerateLMConfig.cmake.in
               ${GenerateLM_BINARY_DIR}/GenerateLMConfig.cmake @ONLY IMMEDIATE)



# Settings specific for installation trees
#
#
SET(GenerateLM_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/include/GenerateLM)
SET(GenerateLM_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateLM)
SET(GenerateLM_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateLM/UseGenerateLM.cmake)
SET(GENERATELM_EXE_CONFIG ${CMAKE_INSTALL_PREFIX}/bin/GenerateLM)
SET(LoadableModule_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/LoadableModule)
SET(ITK_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/InsightToolkit)



# Configure GenerateLMConfig.cmake for the install tree.
CONFIGURE_FILE(${GenerateLM_SOURCE_DIR}/GenerateLMConfig.cmake.in
               ${GenerateLM_BINARY_DIR}/install/GenerateLMConfig.cmake @ONLY IMMEDIATE)
