project(SlicerBase)

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)

subdirs(
  Logic
  )

IF(Slicer3_USE_QT)
  subdirs(GUI2)
ENDIF(Slicer3_USE_QT)

subdirs(
  GUI
  CLI
  )
