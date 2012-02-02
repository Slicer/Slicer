
slicerMacroBuildCLI(
  NAME ResampleScalarVectorDWIVolume
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${DiffusionFiltering_SOURCE_DIR}
  ADDITIONAL_SRCS
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/itkWarpTransform3D.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/itkWarpTransform3D.txx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/itkTransformDeformationFieldFilter.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/itkTransformDeformationFieldFilter.txx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/dtiprocessFiles/deformationfieldio.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/dtiprocessFiles/deformationfieldio.cxx
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/dtiprocessFiles/dtitypes.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    ${DiffusionFiltering_SOURCE_DIR}/ResampleDTI/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
