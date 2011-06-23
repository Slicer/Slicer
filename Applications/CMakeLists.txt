
if(Slicer_BUILD_CLI)
  add_subdirectory(CLI)
endif()

add_subdirectory(SlicerQT)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
