project(EMSegmentMRML)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

# --------------------------------------------------------------------------
# Find Slicer3

#if(NOT Slicer_SOURCE_DIR)
#  find_package(Slicer3 REQUIRED)
#  include(${Slicer_USE_FILE})
#  slicer3_set_default_install_prefix_for_external_projects()
#endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLEMSConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLEMSConfigure.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  # For vtkEMSegmentMRMLManager
  ${EMSegmentAlgorithm_SOURCE_DIR}
  ${EMSegmentAlgorithm_BINARY_DIR}
  # For vtkMRMLEMSVolumeCollectionNode
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLEMSConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(KIT_SRCS 
  vtkEMSegmentMRMLManager.cxx
  vtkMRMLEMSNode.cxx
  vtkMRMLEMSSegmenterNode.cxx
  vtkMRMLEMSTemplateNode.cxx
  vtkMRMLEMSTreeNode.cxx
  vtkMRMLEMSTreeParametersNode.cxx
  vtkMRMLEMSTreeParametersParentNode.cxx
  vtkMRMLEMSTreeParametersLeafNode.cxx
  vtkMRMLEMSGlobalParametersNode.cxx
  vtkMRMLEMSVolumeCollectionNode.cxx
  vtkMRMLEMSAtlasNode.cxx
  vtkMRMLEMSTargetNode.cxx
  vtkMRMLEMSWorkingDataNode.cxx
  vtkMRMLEMSIntensityNormalizationParametersNode.cxx
  vtkMRMLEMSClassInteractionMatrixNode.cxx
  )
  
# --------------------------------------------------------------------------
# Wrapping
set(KIT_TCL_SRCS)
if(VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(${PROJECT_NAME}
    KIT_TCL_SRCS
    "${KIT_SRCS}" "")
endif()

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ${PROJECT_NAME})
add_library(${lib_name} 
  ${KIT_SRCS} 
  ${KIT_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${MRML_LIBRARIES}
  ${ITK_LIBRARIES}
  # For vtkEMSegmentMRMLManager
  EMSegmentAlgorithm
  # For vtkMRMLEMSVolumeCollectionNode
  SlicerBaseLogic
  vtkSlicerVolumesModuleLogic
  )

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
    ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

#install(DIRECTORY
#  Testing/TestData
#  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#  PATTERN ".svn" EXCLUDE
#  )
