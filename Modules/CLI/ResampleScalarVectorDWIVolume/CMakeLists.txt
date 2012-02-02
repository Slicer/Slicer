
slicerMacroBuildCLI(
  NAME ResampleScalarVectorDWIVolume
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${DiffusionFiltering_SOURCE_DIR}
  ADDITIONAL_SRCS
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/itkWarpTransform3D.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/itkWarpTransform3D.txx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/itkTransformDeformationFieldFilter.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/itkTransformDeformationFieldFilter.txx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/dtiprocessFiles/deformationfieldio.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/dtiprocessFiles/deformationfieldio.cxx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/dtiprocessFiles/dtitypes.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTIVolume/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
