
#-----------------------------------------------------------------------------
set(MODULE_NAME DiffusionWeightedVolumeMasking) # Do not use 'project()'

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRMLCore vtkTeem SlicerBaseLogic
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${SlicerBaseLogic_SOURCE_DIR}
    ${SlicerBaseLogic_BINARY_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
