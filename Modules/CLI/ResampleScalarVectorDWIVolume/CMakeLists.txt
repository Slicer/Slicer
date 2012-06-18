
#-----------------------------------------------------------------------------
set(MODULE_NAME ResampleScalarVectorDWIVolume) # Do not use 'project()'

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${ResampleDTIVolume_SOURCE_DIR}
  ADDITIONAL_SRCS
    ${ResampleDTIVolume_SOURCE_DIR}/itkWarpTransform3D.h
    ${ResampleDTIVolume_SOURCE_DIR}/itkWarpTransform3D.txx
    ${ResampleDTIVolume_SOURCE_DIR}/itkTransformDeformationFieldFilter.h
    ${ResampleDTIVolume_SOURCE_DIR}/itkTransformDeformationFieldFilter.txx
    ${ResampleDTIVolume_SOURCE_DIR}/dtiprocessFiles/deformationfieldio.h
    ${ResampleDTIVolume_SOURCE_DIR}/dtiprocessFiles/deformationfieldio.cxx
    ${ResampleDTIVolume_SOURCE_DIR}/dtiprocessFiles/dtitypes.h
    ${ResampleDTIVolume_SOURCE_DIR}/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    ${ResampleDTIVolume_SOURCE_DIR}/dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
