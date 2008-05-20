project(SlicerBase)

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)

subdirs(
  Logic
  GUI
  CLI
  )
