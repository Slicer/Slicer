
#-----------------------------------------------------------------------------
set(MODULE_NAME ModelToLabelMap)

#-----------------------------------------------------------------------------
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(VTK_LIBRARIES
    vtkCommonCore
    vtkFiltersModeling
    vtkIOLegacy
    vtkIOXML
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
  TARGET_LIBRARIES ${ITK_LIBRARIES} ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
