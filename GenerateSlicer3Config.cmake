# Generate the Slicer3Config.cmake file in the build tree and configure one
# for the installation tree.  This file tells external project how to use
# Slicer.  This provides a mechanism for third party developers to build 
# modules against a Slicer installation.
#
#

# The configuration process is very different for a build tree and an
# installation. The resulting directory structures are vastly
# different. So, the two configured files not only have different
# settings, they have a different structure.

# Settings that are the same for build trees and installation trees
#
#

# Settings specific to build trees
#
#

SET(Slicer3_USE_FILE_CONFIG ${Slicer3_BINARY_DIR}/UseSlicer3.cmake)
SET(TCLAP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/ModuleDescriptionParser)
SET(LoadableModule_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/LoadableModule)
SET(GenerateCLP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/GenerateCLP)
SET(GenerateCLP_USE_FILE_CONFIG ${Slicer3_BINARY_DIR}/Libs/GenerateCLP/UseGenerateCLP.cmake)
SET(Slicer3_INCLUDE_DIRS_CONFIG ${Slicer3_INCLUDE_DIRS})
SET(Slicer3_LIBRARY_DIRS_CONFIG ${Slicer3_BINARY_DIR}/bin)

SET(ITK_DIR_CONFIG ${ITK_DIR})
SET(VTK_DIR_CONFIG ${VTK_DIR})
SET(KWWidgets_DIR_CONFIG ${KWWidgets_DIR})

# Configure Slicer3Config.cmake for the install tree.
CONFIGURE_FILE(
  ${Slicer3_SOURCE_DIR}/Slicer3Config.cmake.in
  ${Slicer3_BINARY_DIR}/Slicer3Config.cmake @ONLY IMMEDIATE)

# Settings specific for installation trees      
#      
# (Note we configure from a different file than use for the build tree)      

# Configure Slicer3Config.cmake for the install tree.      
CONFIGURE_FILE(
  ${Slicer3_SOURCE_DIR}/Slicer3InstallConfig.cmake.in      
  ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake @ONLY IMMEDIATE)      
