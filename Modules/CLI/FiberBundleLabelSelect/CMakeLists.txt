
#-----------------------------------------------------------------------------
set(MODULE_NAME FiberBundleLabelSelect)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES SlicerBaseLogic vtkTeem
  LINK_DIRECTORIES
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    #EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
#if(BUILD_TESTING)
  #  add_subdirectory(Testing)
  #endif()
