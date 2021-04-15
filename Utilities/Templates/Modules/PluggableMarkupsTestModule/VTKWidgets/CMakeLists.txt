project(vtkSlicer${MODULE_NAME}ModuleVTKWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_VTKWIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
  ${vtkSlicerMarkupsModuleVTKWidgets_INCLUDE_DIRS}
  ${vtkSlicerMarkupsModuleVTKWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicerTestLineWidget.h
  vtkSlicerTestLineWidget.cxx
  vtkSlicerTestLineRepresentation3D.h
  vtkSlicerTestLineRepresentation3D.cxx
  vtkSlicerTestLineRepresentation2D.h
  vtkSlicerTestLineRepresentation2D.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerMarkupsModuleVTKWidgets
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
