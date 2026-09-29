if(Slicer_WITH_LIBRARY_VERSION)
  if(NOT Slicer_VERSION_FULL OR NOT Slicer_VERSION)
    message(FATAL_ERROR "Slicer library version properties require SlicerVersion")
  endif()

  set(Slicer_LIBRARY_PROPERTIES ${Slicer_LIBRARY_PROPERTIES}
    VERSION ${Slicer_VERSION_FULL}
    SOVERSION ${Slicer_VERSION}
    )
endif()
