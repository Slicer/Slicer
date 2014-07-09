
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionTensorScalarMeasurements)

# Avoid vtkNRRDReader/vtkNrrdReader filename confusion on case insensitive file systems
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  MRMLCore vtkTeem ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIRS}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
