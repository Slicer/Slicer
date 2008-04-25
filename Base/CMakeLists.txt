project(SlicerBase)

subdirs(
  Logic
  GUI
  CLI
  )

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)

configure_file(
  ${SlicerBase_SOURCE_DIR}/vtkSlicerConfigure.h.in 
  ${SlicerBase_BINARY_DIR}/vtkSlicerConfigure.h
  )

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${SlicerBase_BINARY_DIR})
