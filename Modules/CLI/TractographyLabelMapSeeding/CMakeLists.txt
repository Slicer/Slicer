
#-----------------------------------------------------------------------------
set(MODULE_NAME TractographyLabelMapSeeding)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES vtkTeem SlicerBaseLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
