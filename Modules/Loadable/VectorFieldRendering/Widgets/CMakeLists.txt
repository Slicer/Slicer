project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerColorsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerColorsModuleLogic_INCLUDE_DIRS}
  ${qSlicerColorsModuleWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLColorLegendGroupBox.cxx
  qMRMLColorLegendGroupBox.h
  qMRMLVectorFieldDisplayWidget.cxx
  qMRMLVectorFieldDisplayWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLVectorFieldDisplayWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerColorsModuleMRML
  vtkSlicerColorsModuleLogic
  qSlicerColorsModuleWidgets
  qMRMLWidgets
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()
