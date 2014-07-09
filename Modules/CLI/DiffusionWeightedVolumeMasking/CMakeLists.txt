
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionWeightedVolumeMasking)

# Avoid vtkNRRDReader/vtkNrrdReader filename confusion on case insensitive file systems
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  MRMLCore vtkTeem vtkSlicerEditorLibModuleLogic ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${vtkSlicerEditorLibModuleLogic_SOURCE_DIR}
    ${vtkSlicerEditorLibModuleLogic_BINARY_DIR}
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
