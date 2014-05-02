project(vtkSlicer${MODULE_NAME}ModuleVTKWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_VTKWIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtk${MODULE_NAME}GlyphSource2D.cxx
  vtk${MODULE_NAME}GlyphSource2D.h
  )
if(${VTK_VERSION_MAJOR} GREATER 5)
  set(${KIT}_TARGET_LIBRARIES
    vtkSlicer${MODULE_NAME}ModuleMRML
    )
else()
  set(${KIT}_TARGET_LIBRARIES
    vtkRendering
    vtkSlicer${MODULE_NAME}ModuleMRML
  )
endif()

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
