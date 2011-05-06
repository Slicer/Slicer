# Generate the SlicerConfig.cmake file in the build tree and configure one
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

SET(Slicer_USE_FILE_CONFIG ${Slicer_BINARY_DIR}/UseSlicer.cmake)

# License and Readme file
SET(Slicer_LICENSE_FILE_CONFIG ${Slicer_SOURCE_DIR}/License.txt)
SET(Slicer_README_FILE_CONFIG ${Slicer_SOURCE_DIR}/README.txt)

# Path to extension CPack script
set(Slicer_EXTENSION_CPACK_CONFIG ${Slicer_SOURCE_DIR}/SlicerExtensionCPack.cmake)

SET(Slicer_Libs_INCLUDE_DIRS_CONFIG ${Slicer_Libs_INCLUDE_DIRS})
SET(Slicer_Libs_LIBRARY_DIRS_CONFIG ${Slicer_BINARY_DIR}/bin ${Slicer_BINARY_DIR}/lib)

SET(Slicer_Base_INCLUDE_DIRS_CONFIG ${Slicer_Base_INCLUDE_DIRS})
SET(Slicer_Base_LIBRARY_DIRS_CONFIG ${Slicer_BINARY_DIR}/bin)

SET(Slicer_ModuleLogic_INCLUDE_DIRS_CONFIG ${Slicer_ModuleLogic_INCLUDE_DIRS})
SET(Slicer_ModuleLogic_LIBRARY_DIRS_CONFIG ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR})


# Qt
SET(QT_QMAKE_EXECUTABLE_CONFIG ${QT_QMAKE_EXECUTABLE})

# Slicer libraries
SET(TCLAP_DIR_CONFIG ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/tclap)
SET(ModuleDescriptionParser_DIR_CONFIG ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/ModuleDescriptionParser)
SET(GenerateCLP_DIR_CONFIG ${Slicer_BINARY_DIR}/Libs/SlicerExecutionModel/GenerateCLP)

# External project
if(Slicer_USE_BatchMake)
  SET(BatchMake_DIR_CONFIG ${BatchMake_DIR})
endif()
SET(CTK_DIR_CONFIG ${CTK_DIR})
SET(ITK_DIR_CONFIG ${ITK_DIR})
SET(OpenCV_DIR_CONFIG ${OpenCV_DIR})
SET(PYTHON_INCLUDE_PATH_CONFIG ${PYTHON_INCLUDE_PATH})
SET(PYTHON_LIBRARIES_CONFIG ${PYTHON_LIBRARIES})
SET(Teem_DIR_CONFIG ${Teem_DIR})
SET(VTK_DIR_CONFIG ${VTK_DIR})

# List all required external project
SET(Slicer_EXTERNAL_PROJECTS_CONFIG CTK GenerateCLP ITK Teem VTK)
IF(Slicer_USE_BatchMake)
  LIST(APPEND Slicer_EXTERNAL_PROJECTS_CONFIG BatchMake)
ENDIF()

# Configure SlicerConfig.cmake for the install tree.
CONFIGURE_FILE(
  ${Slicer_SOURCE_DIR}/SlicerConfig.cmake.in
  ${Slicer_BINARY_DIR}/SlicerConfig.cmake @ONLY)

# Settings specific for installation trees      
#      
# (Note we configure from a different file than use for the build tree)      

# Configure SlicerConfig.cmake for the install tree.      
CONFIGURE_FILE(
  ${Slicer_SOURCE_DIR}/SlicerInstallConfig.cmake.in      
  ${Slicer_BINARY_DIR}/Utilities/SlicerConfig.cmake @ONLY)
