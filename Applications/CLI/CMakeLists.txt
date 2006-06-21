PROJECT( CLI )

IF (NOT Slicer3_SOURCE_DIR)
  FIND_PACKAGE ( ITK )
  IF ( ITK_FOUND )
    INCLUDE(${ITK_USE_FILE})
  ELSE ( ITK_FOUND )
    MESSAGE ( FATAL_ERROR "Cannot build without ITK" )
  ENDIF ( ITK_FOUND )
ENDIF (NOT Slicer3_SOURCE_DIR)

INCLUDE(../../Libs/GenerateCLP/UseGenerateCLP.cmake)

SET ( ResampleVolume_SOURCE ResampleVolume.cxx )
GENERATECLP(ResampleVolume_SOURCE ResampleVolume.xml)
ADD_EXECUTABLE ( ResampleVolume ${ResampleVolume_SOURCE})
TARGET_LINK_LIBRARIES ( ResampleVolume ITKIO ITKBasicFilters ITKCommon)

SET ( ImageReadDicomWrite_SOURCE ImageReadDicomWrite.cxx )
GENERATECLP(ImageReadDicomWrite_SOURCE ImageReadDicomWrite.xml)
ADD_EXECUTABLE ( ImageReadDicomWrite ${ImageReadDicomWrite_SOURCE})
TARGET_LINK_LIBRARIES ( ImageReadDicomWrite ITKIO ITKBasicFilters ITKCommon)

SET ( GradientAnisotropicDiffusion_SOURCE GradientAnisotropicDiffusion.cxx )
GENERATECLP(GradientAnisotropicDiffusion_SOURCE GradientAnisotropicDiffusion.xml)
ADD_EXECUTABLE ( GradientAnisotropicDiffusion ${GradientAnisotropicDiffusion_SOURCE})
TARGET_LINK_LIBRARIES ( GradientAnisotropicDiffusion ITKIO ITKBasicFilters ITKCommon)

SET ( MedianImageFilter_SOURCE MedianImageFilter.cxx )
GENERATECLP(MedianImageFilter_SOURCE MedianImageFilter.xml)
ADD_EXECUTABLE ( MedianImageFilter ${MedianImageFilter_SOURCE})
TARGET_LINK_LIBRARIES ( MedianImageFilter ITKIO ITKBasicFilters ITKCommon)

SET ( ExecutionModelTour_SOURCE ExecutionModelTour.cxx )
GENERATECLP(ExecutionModelTour_SOURCE ExecutionModelTour.xml)
ADD_EXECUTABLE ( ExecutionModelTour ${ExecutionModelTour_SOURCE})
TARGET_LINK_LIBRARIES ( ExecutionModelTour ITKIO ITKBasicFilters ITKCommon)

#IF (Slicer3_BUILD_TESTING)
#  SUBDIRS( Testing )
#ENDIF (Slicer3_BUILD_TESTING)
