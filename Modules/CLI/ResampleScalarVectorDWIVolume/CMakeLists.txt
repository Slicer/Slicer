
#-----------------------------------------------------------------------------
set(MODULE_NAME ResampleScalarVectorDWIVolume)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOImageBase
  ITKIOTransformBase
  ITKImageFunction
  ITKImageGrid
  ITKTransform
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
include(${ITK_USE_FILE})

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
