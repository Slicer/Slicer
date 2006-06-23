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

SET ( OtsuThresholdImageFilter_SOURCE OtsuThresholdImageFilter.cxx )
GENERATECLP(OtsuThresholdImageFilter_SOURCE OtsuThresholdImageFilter.xml)
ADD_EXECUTABLE ( OtsuThresholdImageFilter ${OtsuThresholdImageFilter_SOURCE})
TARGET_LINK_LIBRARIES ( OtsuThresholdImageFilter ITKIO ITKBasicFilters ITKCommon)

SET ( OtsuThresholdSegmentation_SOURCE OtsuThresholdSegmentation.cxx )
GENERATECLP(OtsuThresholdSegmentation_SOURCE OtsuThresholdSegmentation.xml)
ADD_EXECUTABLE ( OtsuThresholdSegmentation ${OtsuThresholdSegmentation_SOURCE})
TARGET_LINK_LIBRARIES ( OtsuThresholdSegmentation ITKIO ITKBasicFilters ITKCommon)

SET ( VotingBinaryHoleFillingImageFilter_SOURCE VotingBinaryHoleFillingImageFilter.cxx )
GENERATECLP(VotingBinaryHoleFillingImageFilter_SOURCE VotingBinaryHoleFillingImageFilter.xml)
ADD_EXECUTABLE ( VotingBinaryHoleFillingImageFilter ${VotingBinaryHoleFillingImageFilter_SOURCE})
TARGET_LINK_LIBRARIES ( VotingBinaryHoleFillingImageFilter ITKIO ITKBasicFilters ITKCommon)

SET ( ExecutionModelTour_SOURCE ExecutionModelTour.cxx )
GENERATECLP(ExecutionModelTour_SOURCE ExecutionModelTour.xml)
ADD_EXECUTABLE ( ExecutionModelTour ${ExecutionModelTour_SOURCE})
TARGET_LINK_LIBRARIES ( ExecutionModelTour ITKIO ITKBasicFilters ITKCommon)

SET ( ModelMaker_SOURCE ModelMaker.cxx)
GENERATECLP(ModelMaker_SOURCE ModelMaker.xml)
ADD_EXECUTABLE(ModelMaker ${ModelMaker_SOURCE})
TARGET_LINK_LIBRARIES (ModelMaker vtkITK)
INCLUDE_DIRECTORIES(${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR})

#IF (Slicer3_BUILD_TESTING)
#  SUBDIRS( Testing )
#ENDIF (Slicer3_BUILD_TESTING)
