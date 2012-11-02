
#-----------------------------------------------------------------------------
set(MODULE_NAME GrayscaleModelMaker) # Do not use 'project()'

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser MRMLCore SlicerBaseCLI
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
