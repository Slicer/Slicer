
#-----------------------------------------------------------------------------
set(MODULE_NAME DicomToNrrdConverter) # Do not use 'project()'

foreach(varname ITK_USE_REVIEW ITK_USE_REVIEW_STATISTICS ITK_USE_OPTIMIZED_REGISTRATION_METHODS)
  if(NOT ${varname})
    message(FATAL_ERROR "ITK was not built with ${varname} option enabled")
  endif()
endforeach()

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  ## There's limited testing from within Slicer.  There's extensive testing when run
  ## at UIowa but the testing data isn't publicly accessible yet.  Other packages that
  ## use this tool will not run either set of tests.
  add_subdirectory(Testing)

  set(RUN_EXTENDED_LOCAL_TESTING OFF CACHE BOOL "Run extended tests?")
  if(RUN_EXTENDED_LOCAL_TESTING)
    add_subdirectory(ExtendedTesting)
  endif()
endif()
