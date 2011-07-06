
slicerMacroBuildCLI(
  NAME Multiply
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

