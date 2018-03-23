
#-----------------------------------------------------------------------------
set(MODULE_NAME ProbeVolumeWithModel)

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  vtkTeem
  ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
