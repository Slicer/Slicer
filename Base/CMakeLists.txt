PROJECT( SlicerBase )

SUBDIRS(
    Logic
    GUI
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

CONFIGURE_FILE(
  ${SlicerBase_SOURCE_DIR}/vtkSlicerConfigure.h.in 
  ${SlicerBase_BINARY_DIR}/vtkSlicerConfigure.h
)
