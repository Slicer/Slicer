
#-----------------------------------------------------------------------------
set(MODULE_NAME DTIimport) # Do not use 'project()'

SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
  ${ITK_SOURCE_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
set(MODULE_NAME DTIexport) # Do not use 'project()'

SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  INCLUDE_DIRECTORIES
  ${ITK_SOURCE_DIR}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
