
#-----------------------------------------------------------------------------
set(MODULE_NAME TractographyLabelMapSeeding)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES vtkTeem SlicerBaseLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR}
    ${vtkITK_BINARY_DIR}
    ${vtkTeem_SOURCE_DIR}
    ${vtkTeem_BINARY_DIR}
    #  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
