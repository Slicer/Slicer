
if(Slicer_BUILD_QTLOADABLEMODULES)
  add_subdirectory(Loadable)
endif()

if(Slicer_BUILD_QTSCRIPTEDMODULES)
  add_subdirectory(Scripted)
endif()

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI)
  add_subdirectory(CLI)
endif()
