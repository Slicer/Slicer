
slicerMacroBuildCLI(
  NAME MergeModels
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES vtkIO vtkGraphics
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

