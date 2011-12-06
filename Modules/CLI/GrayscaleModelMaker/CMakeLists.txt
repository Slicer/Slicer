
slicerMacroBuildCLI(
  NAME GrayscaleModelMaker
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkITK ModuleDescriptionParser SlicerBaseCLI vtkHybrid
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR}
  EXECUTABLE_ONLY
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

