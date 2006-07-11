PROJECT( CLI )

IF (NOT Slicer3_SOURCE_DIR)
  FIND_PACKAGE ( ITK )
  IF ( ITK_FOUND )
    INCLUDE(${ITK_USE_FILE})
  ELSE ( ITK_FOUND )
    MESSAGE ( FATAL_ERROR "Cannot build without ITK" )
  ENDIF ( ITK_FOUND )
ENDIF (NOT Slicer3_SOURCE_DIR)

INCLUDE(${Slicer3_SOURCE_DIR}/Libs/GenerateCLP/UseGenerateCLP.cmake)

#####################
SET (CLP LinearRegistration)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

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
SET (CLP GradientAnisotropicDiffusion)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

#####################
SET (CLP MedianImageFilter)

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} ITKIO ITKBasicFilters)

ADD_LIBRARY(${CLP}Lib SHARED ${${CLP}_SOURCE})
SET_TARGET_PROPERTIES (${CLP}Lib PROPERTIES COMPILE_FLAGS "-Dmain=SlicerModuleEntryPoint")
TARGET_LINK_LIBRARIES (${CLP}Lib ITKIO ITKBasicFilters)

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

SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)
ADD_EXECUTABLE(${CLP} ${${CLP}_SOURCE})
TARGET_LINK_LIBRARIES (${CLP} vtkITK vtkGraphics vtkIO)
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


# install each target in the production area (where it would appear in an 
# installation) and install each target in the developer area (for running 
# from a build)
SET(TARGETS
        ResampleVolume
        ResampleVolumeLib
        ExecutionModelTour
        ImageReadDicomWrite
        ImageReadDicomWriteLib
        GradientAnisotropicDiffusion
        GradientAnisotropicDiffusionLib
        MedianImageFilter 
        MedianImageFilterLib
        OtsuThresholdImageFilter
        OtsuThresholdImageFilterLib
        OtsuThresholdSegmentation
        OtsuThresholdSegmentationLib
        VotingBinaryHoleFillingImageFilter
        VotingBinaryHoleFillingImageFilterLib
        ModelMaker
        GrayscaleFillHoleImageFilter
        GrayscaleFillHoleImageFilterLib
        GrayscaleGrindPeakImageFilter
        GrayscaleGrindPeakImageFilterLib
)

FOREACH(target ${TARGETS})
  INSTALL(TARGETS ${target} RUNTIME DESTINATION Plugins
                            LIBRARY DESTINATION Plugins)
ENDFOREACH(target)

