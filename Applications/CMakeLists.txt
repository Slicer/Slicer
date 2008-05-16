project(Applications)

if(BUILD_CLI)
  subdirs(CLI)
endif(BUILD_CLI)

subdirs(
  GUI
  )

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)
