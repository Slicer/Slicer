project(CLI)

# GenerateCLP
find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

# ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# vtkITK
find_package(vtkITK REQUIRED)

include_directories(${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR})
link_directories(${SlicerBaseCLI_BINARY_DIR} ${ModuleDescriptionParser_BINARY_DIR})

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  slicerMacroBuildCLI(
    NAME CLIModule4Test
    LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
    )
endif()
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME DiffusionTensorTest
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO
  )
#-----------------------------------------------------------------------------
#slicerMacroBuildCLI(
#  NAME DiffusionWeightedTest
#  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
#  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters)
#  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME DiffusionTensorEstimation
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem SlicerBaseLogic
  LINK_DIRECTORIES 
    ${MRML_LIB_DIR} 
    ${vtkTeem_LIB_DIR} 
    ${SlicerBaseLogic_SOURCE_DIR} 
    ${SlicerBaseLogic_BINARY_DIR}
  INCLUDE_DIRECTORIES 
    ${vtkITK_SOURCE_DIR} 
    ${vtkITK_BINARY_DIR} 
    ${MRML_SOURCE_DIR} 
    ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR} 
    ${vtkTeem_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME DiffusionTensorMathematics
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRML vtkTeem
  LINK_DIRECTORIES 
    ${MRML_LIB_DIR} 
    ${vtkTeem_LIB_DIR}
  INCLUDE_DIRECTORIES 
    ${MRML_SOURCE_DIR}    ${MRML_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR} ${vtkTeem_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME OrientImage
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME LinearRegistration
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters ITKNumerics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME RigidRegistration
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters ITKNumerics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME AffineRegistration
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters ITKNumerics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME BSplineDeformableRegistration
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters ITKNumerics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME TestGridTransformRegistration
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKStatistics ITKIO ITKBasicFilters ITKNumerics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME CheckerBoard
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ResampleVolume
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME PolyDataToLabelmap
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKIO vtkIO vtkGraphics ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME GaussianBlurImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ConfidenceConnected
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ExecutionModelTour
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/BillsLogo.h
  TARGET_LIBRARIES MRML
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ImageReadDicomWrite
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME CurvatureAnisotropicDiffusion
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME GradientAnisotropicDiffusion
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME MedianImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
add_subdirectory(DicomToNrrdConverter)
#-----------------------------------------------------------------------------
# set (CLP DWIDicomLoad)

# set ( ${CLP}_SOURCE ${CLP}.cxx)
# generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
# add_executable(${CLP} ${${CLP}_SOURCE})
# slicer3_set_plugins_output_path(${CLP})
# target_link_libraries (${CLP} ITKIO ITKBasicFilters)

# add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
# slicer3_set_plugins_output_path(${CLP}Lib)
# set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
# target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME HistogramMatching
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters ITKStatistics
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME OtsuThresholdImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME OtsuThresholdSegmentation
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Subtract
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Multiply
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Add
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Threshold
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Mask
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME Cast
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME VotingBinaryHoleFillingImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
# TODO: Remove SlicerBaseLogic once pass in color node information
slicerMacroBuildCLI(
  NAME ModelMaker
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRML SlicerBaseLogic SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}          ${vtkITK_BINARY_DIR}
    ${MRML_SOURCE_DIR}            ${MRML_BINARY_DIR} 
    ${SlicerBaseLogic_SOURCE_DIR} ${SlicerBaseLogic_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
# TODO: Remove SlicerBaseLogic once pass in color node information
slicerMacroBuildCLI(
  NAME MultipleModelsExample
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRML SlicerBaseLogic SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}          ${vtkITK_BINARY_DIR}
    ${MRML_SOURCE_DIR}            ${MRML_BINARY_DIR} 
    ${SlicerBaseLogic_SOURCE_DIR} ${SlicerBaseLogic_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME GrayscaleModelMaker
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser SlicerBaseCLI vtkHybrid
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME MergeModels
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkIO vtkGraphics
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME GrayscaleFillHoleImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME GrayscaleGrindPeakImageFilter
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME LabelMapSmoothing
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKAlgorithms ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ImageLabelCombine
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ComputeSUVBodyWeight
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h 
  # LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/CTSCLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRML SlicerBaseLogic SlicerBaseCLI
  LINK_DIRECTORIES ${vtkITK_LIB_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}           ${vtkITK_BINARY_DIR} 
    ${MRML_SOURCE_DIR}             ${MRML_BINARY_DIR} 
    ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR}
  )
#-----------------------------------------------------------------------------
slicerMacroBuildCLI(
  NAME ResampleVolume2
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ITKIO ITKBasicFilters ITKAlgorithms
  ADDITIONAL_SRCS
    DiffusionApplications/ResampleDTI/itkWarpTransform3D.h
    DiffusionApplications/ResampleDTI/itkWarpTransform3D.txx
    DiffusionApplications/ResampleDTI/itkTransformDeformationFieldFilter.h
    DiffusionApplications/ResampleDTI/itkTransformDeformationFieldFilter.txx
    DiffusionApplications/ResampleDTI/dtiprocessFiles/deformationfieldio.h
    DiffusionApplications/ResampleDTI/dtiprocessFiles/deformationfieldio.cxx
    DiffusionApplications/ResampleDTI/dtiprocessFiles/dtitypes.h
    DiffusionApplications/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    DiffusionApplications/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  )
#-----------------------------------------------------------------------------
# culled from Slicer 3.6 release
#slicerMacroBuildCLI(
#  NAME ZeroCrossingBasedEdgeDetectionImageFilter
#  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
#  TARGET_LIBRARIES ITKIO ITKBasicFilters
#  )
#-----------------------------------------------------------------------------
# culled from Slicer 3.6 release
#slicerMacroBuildCLI(
#  NAME FreesurferSurfaceSectionExtraction
#  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
#  TARGET_LIBRARIES ITKIO ITKBasicFilters
#  )
###################

SET(cli_modules
  MRIBiasFieldCorrection
  SparseFieldLevelSetContour
  Realign
  DiffusionApplications
  ExtractSkeleton
  Java
  N4ITKBiasFieldCorrection
  RobustStatisticsSegmenter
  RegisterImagesModule
  )

if(Slicer_USE_MIDAS)
  LIST(APPEND cli_modules MIDASApplications)
endif()

if(Slicer_USE_BatchMake)
  LIST(APPEND cli_modules BatchMakeApplications)
endif()

FOREACH(cli_module ${cli_modules})
  ADD_SUBDIRECTORY(${cli_module})
ENDFOREACH(cli_module)

option(BUILD_BRAINSTOOLS "Build the BRAINS subset of registration and segmentation tools" ON)
if(BUILD_BRAINSTOOLS)
  add_subdirectory(BRAINSTools)
endif()

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()
