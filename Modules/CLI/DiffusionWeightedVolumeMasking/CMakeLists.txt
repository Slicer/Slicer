
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionWeightedVolumeMasking)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRMLCore vtkTeem vtkSlicerEditorLibModuleLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${vtkSlicerEditorLibModuleLogic_SOURCE_DIR}
    ${vtkSlicerEditorLibModuleLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
