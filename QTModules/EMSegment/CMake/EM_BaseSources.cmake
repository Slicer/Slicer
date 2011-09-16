#
# Create the EMSegment_BASE_SRCS CMake variable including files for
#    MRML, Algorithms, ImageFilters, AMF, Registration
# and the
#    logic-class, MRMLManager, SlicerCommonInterface
#
# Also, define the appropriate include_directories for these source files
#

set(EMSegment_BASE_SRCS 
  # MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSSegmenterNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTemplateNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersParentNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTreeParametersLeafNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSGlobalParametersNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSCollectionNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSVolumeCollectionNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSAtlasNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSTargetNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSWorkingDataNode.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSIntensityNormalizationParametersNode.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLEMSClassInteractionMatrixNode.cxx

  # Algorithm 
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalInterface.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalRegistrationCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/EMLocalShapeCostFunction.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkDataDef.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkFileOps.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGeneral.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalGenericClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSegmenter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkImageEMLocalSuperClass.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/vtkTimeDef.cxx

  # ImageFilters
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters/vtkSimonParameterReaderWriter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters/vtkImageMeanIntensityNormalization.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters/vtkImageLabelPropagation.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters/vtkImageIslandFilter.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageFilters/vtkImageSumOverVoxels.cxx

  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImageLogOdds.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImageMultiLevelSets.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImageLevelSets.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkLevelSetFastMarching.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImageFastSignedChamfer.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImageIsoContourDist.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImagePropagateDist.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkImagePropagateDist2.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AMF/vtkLevelSets.cxx

  # Registration
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkRigidRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkBSplineRegistrator.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Registration/vtkITKTransformAdapter.cxx

  # not used in current implementation---used in slicer2 modules
  ${CMAKE_CURRENT_SOURCE_DIR}/Tasks/vtkCTHandBoneClass.cxx

  # module
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentMRMLManager.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentLogic.cxx 
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerCommonInterface.cxx
)

include_directories(
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${EMSegment_SOURCE_DIR}/Algorithm/LLSBiasCorrector
  ${EMSegment_SOURCE_DIR}/ImageFilters
  ${EMSegment_SOURCE_DIR}/Registration
  ${EMSegment_SOURCE_DIR}/AMF
  ${EMSegment_SOURCE_DIR}/Tasks
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  ${AtlasCreatorCxx_SOURCE_DIR}
  ${AtlasCreatorCxx_BINARY_DIR}  
  )
