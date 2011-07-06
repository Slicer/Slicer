
slicerMacroBuildCLI(
  NAME PolyDataToLabelmap
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES} vtkIO vtkGraphics ${ITK_LIBRARIES}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

