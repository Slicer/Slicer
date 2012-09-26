# "Core" folder is included in the top-level CMakeLists.txt just before "Base/QTApp". Doing so
# make it possible for QTApp to use on the Core module factory.

if(Slicer_BUILD_QTLOADABLEMODULES)
  add_subdirectory(Loadable)
endif()

if(Slicer_BUILD_QTSCRIPTEDMODULES)
  add_subdirectory(Scripted)
endif()

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI)
  add_subdirectory(CLI)
endif()
