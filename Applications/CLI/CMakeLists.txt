PROJECT( CLI )

# 

FIND_PACKAGE(GenerateCLP REQUIRED)
FIND_PACKAGE(vtkITK REQUIRED)

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
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)


#####################
SET (CLP LinearRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

#####################
SET (CLP AffineRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

#####################
SET (CLP BSplineDeformableRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKStatistics ITKIO ITKBasicFilters)

#####################
SET (CLP CheckerBoard)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ResampleVolume)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ConfidenceConnected)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP ExecutionModelTour)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

#####################


SET (CLP ImageReadDicomWrite)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP CurvatureAnisotropicDiffusion)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GradientAnisotropicDiffusion)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP MedianImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/ITKLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GEDicom2NRRD)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/NAMICLogo.h)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP HistogramMatching)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters ITKStatistics)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters ITKStatistics)

#####################
SET (CLP OtsuThresholdImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP OtsuThresholdSegmentation)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP Subtract)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP Add)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP VotingBinaryHoleFillingImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################

SET (CLP ModelMaker)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPluginFilterWatcher.cxx )
GENERATECLP(${CLP}_SOURCE ${CLP}.xml ${CLI_SOURCE_DIR}/Logos/NAMICLogo.h)
LINK_DIRECTORIES(${vtkITK_LIB_DIR})
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP}
    vtkITK vtkImaging vtkGraphics vtkIO
    ITKIO
    ModuleDescriptionParser
)
INCLUDE_DIRECTORIES(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

#####################
SET (CLP GrayscaleModelMaker)

SET ( ${CLP}_SOURCE ${CLP}.cxx vtkPluginFilterWatcher.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
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
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP GrayscaleGrindPeakImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)


#IF (Slicer3_BUILD_TESTING)
#  SUBDIRS( Testing )
#ENDIF (Slicer3_BUILD_TESTING)


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
        LinearRegistration
        GrayscaleFillHoleImageFilter
        GrayscaleFillHoleImageFilterLib
        GrayscaleGrindPeakImageFilter
        GrayscaleGrindPeakImageFilterLib
        HistogramMatching
        HistogramMatchingLib
        LinearRegistration
        AffineRegistration
        BSplineDeformableRegistration
  )

  FOREACH(target ${TARGETS})
    INSTALL(TARGETS ${target}
      RUNTIME DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins
      LIBRARY DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR}/Plugins)
  ENDFOREACH(target)

ENDIF (Slicer3_SOURCE_DIR)
