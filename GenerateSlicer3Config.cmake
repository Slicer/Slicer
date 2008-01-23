# Generate the Slicer3Config.cmake file in the build tree and configure one
# for the installation tree.  This file tells external project how to use
# Slicer.  This provides a mechanism for third party developers to build 
# modules against a Slicer installation.
#

# Settings that are the same for build trees and installation trees
#
#



# Settings specific to build trees
#
#

SET(Slicer3_USE_FILE_CONFIG ${Slicer3_BINARY_DIR}/UseSlicer3.cmake)
SET(TCLAP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
SET(GenerateCLP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/GenerateCLP)
SET(GenerateCLP_USE_FILE_CONFIG ${Slicer3_BINARY_DIR}/Libs/GenerateCLP/UseGenerateCLP.cmake)
SET(Slicer3_INCLUDE_DIRS_CONFIG ${Slicer3_SOURCE_DIR}/Applications/CLI)
SET(Slicer3_LIBRARY_DIRS_CONFIG ${Slicer3_BINARY_DIR}/bin)


# Configure Slicer3Config.cmake for the install tree.
CONFIGURE_FILE(${Slicer3_SOURCE_DIR}/Slicer3Config.cmake.in
               ${Slicer3_BINARY_DIR}/Slicer3Config.cmake @ONLY IMMEDIATE)



# Settings specific for installation trees
#
#

SET(Slicer3_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/UseSlicer3.cmake)
SET(TCLAP_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/ModuleDescriptionParser)
SET(GenerateCLP_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/lib/GenerateCLP)
SET(GenerateCLP_USE_FILE_CONFIG ${CMAKE_INSTALL_PREFIX}/Libs/GenerateCLP/UseGenerateCLP.cmake)
SET(Slicer3_INCLUDE_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/Slicer3/)
SET(Slicer3_LIBRARY_DIRS_CONFIG ${CMAKE_INSTALL_PREFIX}/bin)



# Configure Slicer3Config.cmake for the install tree.
CONFIGURE_FILE(${Slicer3_SOURCE_DIR}/Slicer3Config.cmake.in
               ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake @ONLY IMMEDIATE)




