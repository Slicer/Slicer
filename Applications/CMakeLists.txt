project(Applications)

if(Slicer3_BUILD_CLI)
  subdirs(CLI)
endif(Slicer3_BUILD_CLI)

subdirs(
  GUI
  )

IF(Slicer3_USE_QT)
  #SUBDIRS(SlicerQT)
ENDIF(Slicer3_USE_QT)

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)
