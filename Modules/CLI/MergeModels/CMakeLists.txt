
slicerMacroBuildCLI(
  NAME MergeModels
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkIO vtkGraphics
  INCLUDE_DIRECTORIES
    ${vtkITK_SOURCE_DIR} ${vtkITK_BINARY_DIR}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

