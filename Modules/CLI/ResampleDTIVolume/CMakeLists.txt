
#-----------------------------------------------------------------------------
set(MODULE_NAME ResampleDTIVolume)

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
  ITKDiffusionTensorImage
  ITKIOImageBase
  ITKIONRRD
  ITKIOTransformBase
  ITKImageFilterBase
  ITKImageFunction
  ITKImageGrid
  ITKImageIntensity
  ITKSpatialObjects
  ITKTestKernel
  ITKTransform
  ${ModuleDescriptionParser_ITK_COMPONENTS}
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
include(${ITK_USE_FILE})


#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    dtiprocessFiles/deformationfieldio.h
    dtiprocessFiles/deformationfieldio.cxx
    dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.h
    dtiprocessFiles/itkHFieldToDeformationFieldImageFilter.txx
  TARGET_LIBRARIES ModuleDescriptionParser ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${SlicerBaseCLI_SOURCE_DIR} ${SlicerBaseCLI_BINARY_DIR}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
