project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${qSlicerTerminologiesModuleWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLColorLegendDisplayNodeWidget.cxx
  qMRMLColorLegendDisplayNodeWidget.h
  qMRMLColorListView.cxx
  qMRMLColorListView.h
  qMRMLColorPickerWidget.cxx
  qMRMLColorPickerWidget.h
  qMRMLColorTableView.cxx
  qMRMLColorTableView.h
  qSlicerColorTableTerminologyDelegate.cxx
  qSlicerColorTableTerminologyDelegate.h
  qSlicerTerminologyEditorWidget.cxx
  qSlicerTerminologyEditorWidget.h
  qSlicerTerminologyEditorButton.cxx
  qSlicerTerminologyEditorButton.h
  qSlicerTerminologyEditorDialog.cxx
  qSlicerTerminologyEditorDialog.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLColorLegendDisplayNodeWidget.ui
  Resources/UI/qMRMLColorPickerWidget.ui
  Resources/UI/qSlicerTerminologyEditorWidget.ui
  )

set(${KIT}_RESOURCES
  ../Resources/qSlicer${MODULE_NAME}Module.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  qSlicerTerminologiesModuleWidgets
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
