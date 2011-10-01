
slicerMacroBuildCLI(
  NAME ChangeTrackerSurfaceTrackingMetric
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES} ITKQuadEdgeMesh vtkIO vtkGraphics
  )
if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()


