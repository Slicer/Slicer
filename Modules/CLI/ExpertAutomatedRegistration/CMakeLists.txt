
#-----------------------------------------------------------------------------
set(MODULE_NAME ExpertAutomatedRegistration)

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
  ITKImageIntensity
  ITKOptimizers
  ITKRegistrationCommon
  ITKSpatialObjects
  ITKStatistics
  ITKTestKernel
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
    ${CMAKE_CURRENT_SOURCE_DIR}/ITKRegistrationHelper
  )

if(SUPPORT_2D_IMAGES)
  set_target_properties(${CLP}Lib
    PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DSUPPORT_2D_IMAGES" )
endif()

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
