
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionWeightedVolumeMasking)

# HACK until each project find_package's required projects.
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#set(VTK_LIBRARIES)
#if(${VTK_VERSION_MAJOR} GREATER 5)
#  list(APPEND VTK_LIBRARIES
#    vtkImagingMorphological
#    )
#endif()

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
  TARGET_LIBRARIES MRMLCore vtkTeem vtkSlicerEditorLibModuleLogic ${VTK_LIBRARIES}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
