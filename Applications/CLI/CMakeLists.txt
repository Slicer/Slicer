PROJECT( CLI )

# 

FIND_PACKAGE(GenerateCLP REQUIRED)
FIND_PACKAGE(vtkITK REQUIRED)

FIND_PACKAGE(ModuleDescriptionParser REQUIRED)
LINK_DIRECTORIES(${ModuleDescriptionParser_BINARY_DIR})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

INCLUDE(${GenerateCLP_SOURCE_DIR}/UseGenerateCLP.cmake)

IF (Slicer3_SOURCE_DIR)
  SET(LIBRARY_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
  SET(EXECUTABLE_OUTPUT_PATH "${SLICER_LIBRARIES_DIR}/Plugins")
ENDIF (Slicer3_SOURCE_DIR)

#####################
SET (CLP DiffusionTensorTest)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

#####################
SET (CLP DiffusionWeightedTest)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

#####################
SET (CLP DiffusionTensorEstimation)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
LINK_DIRECTORIES(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR} ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR})
TARGET_LINK_LIBRARIES (${CLP} ITKBasicFilters MRML vtkTeem SlicerBaseLogic)
INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR} 
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
        ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
SET (CLP DiffusionTensorMathematics)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
LINK_DIRECTORIES(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR})
TARGET_LINK_LIBRARIES (${CLP} MRML vtkTeem)
INCLUDE_DIRECTORIES(
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
        ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
SET (CLP OrientImage)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters ITKNumerics)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP LinearRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters ITKNumerics)

#####################
SET (CLP AffineRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters ITKNumerics)

#####################
SET (CLP BSplineDeformableRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKStatistics ITKIO ITKBasicFilters ITKNumerics)

#####################
SET (CLP CheckerBoard)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml  ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ResampleVolume)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GaussianBlurImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ConfidenceConnected)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ExecutionModelTour)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/BillsLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters MRML)

#####################


SET (CLP ImageReadDicomWrite)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP (${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP CurvatureAnisotropicDiffusion)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GradientAnisotropicDiffusion)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP MedianImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GEDicom2NRRD)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP HistogramMatching)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters ITKStatistics)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters ITKStatistics)

#####################
SET (CLP OtsuThresholdImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP OtsuThresholdSegmentation)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP Subtract)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP Add)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP VotingBinaryHoleFillingImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################

SET (CLP ModelMaker)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPluginFilterWatcher.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
# TODO: remove SlicerBaseLogic once pass in color node information
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser
    MRML
    SlicerBaseLogic
)
INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR} 
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
        ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})

#####################

SET (CLP MultipleModelsExample)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPluginFilterWatcher.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
# TODO: remove SlicerBaseLogic once pass in color node information
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser
    MRML
    SlicerBaseLogic
)
INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR} 
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
        ${SlicerBaseLogic_SOURCE_DIR}  ${SlicerBaseLogic_BINARY_DIR})


#####################
SET (CLP GrayscaleModelMaker)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPluginFilterWatcher.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser
)
INCLUDE_DIRECTORIES(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

#####################
SET (CLP GrayscaleFillHoleImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GrayscaleGrindPeakImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)


#####################
SET (CLP LabelMapSmoothing)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKAlgorithms ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKAlgorithms ITKBasicFilters)

#####################
SET (CLP ProbeVolumeWithModel)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
LINK_DIRECTORIES(${MRML_LIB_DIR} ${vtkTeem_LIB_DIR})
TARGET_LINK_LIBRARIES (${CLP} ITKBasicFilters MRML vtkTeem)
INCLUDE_DIRECTORIES(
        ${vtkITK_SOURCE_DIR} 
        ${vtkITK_BINARY_DIR} 
        ${MRML_SOURCE_DIR} ${MRML_BINARY_DIR} 
        ${vtkTeem_SOURCE_DIR}  ${vtkTeem_BINARY_DIR})

#####################
SET (CLP ZeroCrossingBasedEdgeDetectionImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${Slicer3_SOURCE_DIR}/Resources/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################


SUBDIRS ( Realign )
SUBDIRS( DiffusionApplications )
SUBDIRS( ExtractSkeleton )

IF(USE_MIDAS)
  SUBDIRS( MIDASApplications )
ENDIF(USE_MIDAS)

IF(USE_BatchMake)
  SUBDIRS( BatchMakeApplications )
ENDIF(USE_BatchMake)

IF (Slicer3_SOURCE_DIR)
  # install each target in the production area (where it would appear in an 
  # installation) and install each target in the developer area (for running 
  # from a build)
  SET(TARGETS
        ResampleVolume
        ResampleVolumeLib
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
        CheckerBoardLib
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
        OrientImage
        OrientImageLib
  )

  INSTALL(TARGETS ${TARGETS}
    RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins COMPONENT RuntimeLibraries)

  INSTALL(FILES itkPluginFilterWatcher.h vtkPluginFilterWatcher.h itkPluginUtilities.h DESTINATION include/Slicer3 COMPONENT Development)

ENDIF (Slicer3_SOURCE_DIR)

#

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)
