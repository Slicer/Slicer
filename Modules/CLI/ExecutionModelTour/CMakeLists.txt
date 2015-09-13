
#-----------------------------------------------------------------------------
set(MODULE_NAME ExecutionModelTour)

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/BillsLogo.h
  TARGET_LIBRARIES
    # ${VTK_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
    MRMLCore
    vtkSlicerMarkupsModuleMRML
  INCLUDE_DIRECTORIES
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkSlicerMarkupsModuleMRML_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
