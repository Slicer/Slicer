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

SET(TCLAP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/ModuleDescriptionParser)
SET(GenerateCLP_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/GenerateCLP)
SET(Slicer3_USE_FILE_CONFIG ${Slicer3_BINARY_DIR}/UseSlicer3.cmake)
SET(LoadableModule_DIR_CONFIG ${Slicer3_BINARY_DIR}/Libs/LoadableModule)

SET(Slicer3_Libs_INCLUDE_DIRS_CONFIG ${Slicer3_Libs_INCLUDE_DIRS})
SET(Slicer3_Libs_LIBRARY_DIRS_CONFIG ${Slicer3_BINARY_DIR}/bin ${Slicer3_BINARY_DIR}/lib)

SET(Slicer3_Base_INCLUDE_DIRS_CONFIG ${Slicer3_Base_INCLUDE_DIRS})
SET(Slicer3_Base_LIBRARY_DIRS_CONFIG ${Slicer3_BINARY_DIR}/bin)

SET(ITK_DIR_CONFIG ${ITK_DIR})
SET(VTK_DIR_CONFIG ${VTK_DIR})
IF (Slicer3_USE_KWWIDGETS)
  SET(KWWidgets_DIR_CONFIG ${KWWidgets_DIR})
ENDIF (Slicer3_USE_KWWIDGETS)
SET(Teem_DIR_CONFIG ${Teem_DIR})
SET(PYTHON_INCLUDE_PATH_CONFIG ${PYTHON_INCLUDE_PATH})
SET(PYTHON_LIBRARIES_CONFIG ${PYTHON_LIBRARIES})
SET(BatchMake_DIR_CONFIG ${BatchMake_DIR})
SET(OpenCV_DIR_CONFIG ${OpenCV_DIR})

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
