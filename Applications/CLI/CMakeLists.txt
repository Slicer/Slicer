project( CLI )

# 

find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)
find_package(vtkITK REQUIRED)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

slicer3_set_default_plugins_output_path()

include_directories( ${SlicerBaseCLI_BINARY_DIR} ${SlicerBaseCLI_SOURCE_DIR} )
link_directories(${SlicerBaseCLI_BINARY_DIR})

#####################
set (CLP DiffusionTensorTest)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

#####################
set (CLP DiffusionWeightedTest)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

#####################
set (CLP DiffusionTensorEstimation)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
link_directories(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR} ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR})
target_link_libraries (${CLP} ITKBasicFilters MRML vtkTeem SlicerBaseLogic)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
set (CLP DiffusionTensorMathematics)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
link_directories(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR})
target_link_libraries (${CLP} MRML vtkTeem)
include_directories(
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
set (CLP OrientImage)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKNumerics)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP LinearRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKNumerics)

#####################
set (CLP AffineRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKNumerics)

#####################
set (CLP BSplineDeformableRegistration)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKStatistics ITKIO ITKBasicFilters ITKNumerics)

#####################
set (CLP CheckerBoard)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml  ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP ResampleVolume)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP GaussianBlurImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP ConfidenceConnected)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP ExecutionModelTour)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/BillsLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters MRML)

#####################


set (CLP ImageReadDicomWrite)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp (${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP CurvatureAnisotropicDiffusion)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP GradientAnisotropicDiffusion)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP MedianImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP DicomToNrrdConverter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP DWIDicomLoad)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP HistogramMatching)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKStatistics)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKStatistics)

#####################
set (CLP OtsuThresholdImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP OtsuThresholdSegmentation)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP Subtract)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP Add)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP VotingBinaryHoleFillingImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################

set (CLP ModelMaker)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
# TODO: remove SlicerBaseLogic once pass in color node information
target_link_libraries (${CLP}
  vtkITK vtkImaging vtkGraphics vtkIO
  ITKIO
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})

#####################

set (CLP MultipleModelsExample)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
# TODO: remove SlicerBaseLogic once pass in color node information
target_link_libraries (${CLP}
  vtkITK vtkImaging vtkGraphics vtkIO
  ITKIO
  ModuleDescriptionParser
  MRML
  SlicerBaseLogic
  SlicerBaseCLI
  )
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})


#####################
set (CLP GrayscaleModelMaker)

set ( ${CLP}_SOURCE ${CLP}.cxx )
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP}
  vtkITK vtkImaging vtkGraphics vtkIO
  ITKIO
  ModuleDescriptionParser
  SlicerBaseCLI
  )
include_directories(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

#####################
set (CLP GrayscaleFillHoleImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################
set (CLP GrayscaleGrindPeakImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)


#####################
set (CLP LabelMapSmoothing)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKAlgorithms ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKAlgorithms ITKBasicFilters)

#####################
set (CLP ImageLabelCombine)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
link_directories(${vtkTeem_LIB_DIR})
target_link_libraries (${CLP} vtkITK vtkImaging vtkGraphics vtkIO vtkTeem)
include_directories(
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib vtkITK vtkImaging vtkGraphics vtkIO vtkTeem)
include_directories(
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
set (CLP ProbeVolumeWithModel)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
link_directories(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR})
target_link_libraries (${CLP} ITKBasicFilters MRML vtkTeem)
include_directories(
  ${vtkITK_SOURCE_DIR} 
  ${vtkITK_BINARY_DIR} 
  ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
  ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})
#####################
set (CLP ResampleVolume2)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKCommon ITKAlgorithms)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters ITKCommon ITKAlgorithms)

#####################
set (CLP ZeroCrossingBasedEdgeDetectionImageFilter)

set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters)

add_library(${CLP}Lib SHARED ${${CLP}_SOURCE})
set_target_properties (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
target_link_libraries (${CLP}Lib ITKIO ITKBasicFilters)

#####################


set (CLP FreesurferSurfaceSectionExtraction)
set ( ${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml)

add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries (${CLP} ITKIO ITKBasicFilters ITKCommon)

set( Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

###################

subdirs ( Realign )
subdirs( DiffusionApplications )
subdirs( ExtractSkeleton )
#SUBDIRS( RegisterImagesModule )
subdirs( UtahBSpline )

if(Slicer3_USE_MIDAS)
  subdirs( MIDASApplications )
endif(Slicer3_USE_MIDAS)

if(Slicer3_USE_BatchMake)
  subdirs( BatchMakeApplications )
endif(Slicer3_USE_BatchMake)

if (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  set(TARGETS
    ResampleVolume
    ResampleVolumeLib
    ResampleVolume2
    ResampleVolume2Lib
    ExecutionModelTour
    ImageReadDicomWrite
    ImageReadDicomWriteLib
    Add
    AddLib
    ConfidenceConnected
    ConfidenceConnectedLib
    CurvatureAnisotropicDiffusion
    CurvatureAnisotropicDiffusionLib
    GradientAnisotropicDiffusion
    GradientAnisotropicDiffusionLib
    MedianImageFilter 
    MedianImageFilterLib
    OtsuThresholdImageFilter
    OtsuThresholdImageFilterLib
    OtsuThresholdSegmentation
    OtsuThresholdSegmentationLib
    Subtract
    SubtractLib
    VotingBinaryHoleFillingImageFilter
    VotingBinaryHoleFillingImageFilterLib
    ModelMaker
    GrayscaleModelMaker
    GrayscaleFillHoleImageFilter
    GrayscaleFillHoleImageFilterLib
    GrayscaleGrindPeakImageFilter
    GrayscaleGrindPeakImageFilterLib
    HistogramMatching
    HistogramMatchingLib
    LinearRegistration
    AffineRegistration
    BSplineDeformableRegistration
    LabelMapSmoothing
    LabelMapSmoothingLib
    ImageLabelCombine
    ImageLabelCombineLib
    OrientImage
    OrientImageLib
    CheckerBoardLib
    GaussianBlurImageFilter
    GaussianBlurImageFilterLib
    DicomToNrrdConverter
    DicomToNrrdConverterLib
    DWIDicomLoad
    DWIDicomLoadLib
    FreesurferSurfaceSectionExtraction
    ZeroCrossingBasedEdgeDetectionImageFilter
    ZeroCrossingBasedEdgeDetectionImageFilterLib
    ProbeVolumeWithModel
    DiffusionTensorMathematics
    DiffusionTensorEstimation
    )


  install(TARGETS ${TARGETS}
        RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
        LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
        )

endif (Slicer3_SOURCE_DIR)

#

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)
