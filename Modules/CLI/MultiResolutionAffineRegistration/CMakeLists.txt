
#-----------------------------------------------------------------------------
set(MODULE_NAME MultiResolutionAffineRegistration)

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
  ITKImageGrid
  ITKOptimizers
  ITKRegistrationCommon
  ITKReview
  ITKSpatialObjects
  ITKTransform
  )
find_package(ITK 4.5 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
include(${ITK_USE_FILE})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ADDITIONAL_SRCS
    ScalingHeuristics.cxx
    ConvertSlicerROIToRegion.cxx
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
