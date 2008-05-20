project(EMSegment)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_modules_output_path()

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Wizard/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MRML/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/*.txx"
  "${CMAKE_CURRENT_SOURCE_DIR}/Registration/*.h"
  )
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(EMSegment_SRCS 
  # module
  vtkEMSegmentGUI.cxx
  vtkEMSegmentLogic.cxx 
  vtkEMSegmentMRMLManager.cxx 
  # wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentParametersSetStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentIntensityImagesStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentIntensityNormalizationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentAnatomicalStructureStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentSpatialPriorsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentNodeParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentIntensityDistributionsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentRegistrationParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentRunSegmentationStep.cxx
  # MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSSegmenterNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTemplateNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersParentNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersLeafNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSGlobalParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSVolumeCollectionNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSAtlasNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTargetNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSWorkingDataNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSIntensityNormalizationParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSClassInteractionMatrixNode.cxx
  # Algorithm 
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalInterface.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalRegistrationCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalShapeCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkDataDef.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkFileOps.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGeneral.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSegmenter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSuperClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkSimonParameterReaderWriter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkTimeDef.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageMeanIntensityNormalization.cxx
  # Registration
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkRigidRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkBSplineRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkITKTransformAdapter.cxx
  # not used in current implementation---used in slicer2 modules
  #  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMAtlasClass.cxx
  #  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMAtlasSegmenter.cxx
  #  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMAtlasSuperClass.cxx
  #  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMMarkov.cxx
  )

set_source_files_properties(
  # Algorithm sources (temporary exclusion)
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSSegmenterNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTemplateNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersParentNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersLeafNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSGlobalParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSVolumeCollectionNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSAtlasNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTargetNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSWorkingDataNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSIntensityNormalizationParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSClassInteractionMatrixNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalInterface.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalRegistrationCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalShapeCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkDataDef.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkFileOps.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGeneral.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSegmenter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSuperClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkSimonParameterReaderWriter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkTimeDef.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageMeanIntensityNormalization.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkBSplineRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkRigidRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkITKTransformAdapter.cxx
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(EMSegment 
  EMSegment_TCL_SRCS 
  "${EMSegment_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(EMSegment_SRCS EMSegment.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name EMSegment)
add_library(${lib_name} 
  ${EMSegment_SRCS} 
  ${EMSegment_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  Volumes
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Command line application

subdirs(CommandLineApplication)

# --------------------------------------------------------------------------
# Install the testing data

install(DIRECTORY
  Testing/TestData
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
