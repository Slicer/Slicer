PROJECT(EMSegment)

#
# Sources
#
SET(EMSegment_SRCS 
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

SET_SOURCE_FILES_PROPERTIES(
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

#
# Include dirs
#
INCLUDE_DIRECTORIES(
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}/Wizard
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/Registration
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(EMSegment EMSegment_TCL_SRCS "${EMSegment_SRCS}" "")

#GENERATELM(EMSegment_SRCS EMSegment.txt)

#
# Build the library
#
ADD_LIBRARY(EMSegment 
  ${EMSegment_SRCS} 
  ${EMSegment_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS EMSegment
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(EMSegment
  SlicerBaseLogic 
  SlicerBaseGUI
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES} 
  Volumes
  )

#
# Testing
#
IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

#
# command line application
#
SUBDIRS(CommandLineApplication)

#
# Configure include file
#
CONFIGURE_FILE(
  ${EMSegment_SOURCE_DIR}/vtkEMSegmentConfigure.h.in 
  ${EMSegment_BINARY_DIR}/vtkEMSegmentConfigure.h
  )
