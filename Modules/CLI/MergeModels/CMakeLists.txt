
#-----------------------------------------------------------------------------
set(MODULE_NAME MergeModels)

#-----------------------------------------------------------------------------
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(VTK_LIBRARIES
    vtkCommonCore
    vtkFiltersCore
    vtkIOLegacy
    vtkIOXML
    vtksys
    )
else()
  set(VTK_LIBRARIES
    vtkIO
    vtkGraphics
    )
endif()
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${VTK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
