
#-----------------------------------------------------------------------------
set(MODULE_NAME DWIToDTIEstimation)

# HACK until each project find_package's required projects.
include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES MRMLCore vtkTeem
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
    ${MRMLCore_INCLUDE_DIRS}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
