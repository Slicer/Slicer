project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRML${MODULE_NAME}DisplayNodeWidget.cxx
  qMRML${MODULE_NAME}DisplayNodeWidget.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.cxx
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.h
  qMRML${MODULE_NAME}InteractionHandleWidget.cxx
  qMRML${MODULE_NAME}InteractionHandleWidget.h
  qMRML${MODULE_NAME}ToolBar.cxx
  qMRML${MODULE_NAME}ToolBar.h
  qMRML${MODULE_NAME}ToolBar_p.h
  qMRML${MODULE_NAME}PlaneWidget.cxx
  qMRML${MODULE_NAME}PlaneWidget.h
  qMRML${MODULE_NAME}ROIWidget.cxx
  qMRML${MODULE_NAME}ROIWidget.h
  qMRML${MODULE_NAME}AbstractOptionsWidget.cxx
  qMRML${MODULE_NAME}AbstractOptionsWidget.h
  qMRML${MODULE_NAME}AngleMeasurementsWidget.h
  qMRML${MODULE_NAME}AngleMeasurementsWidget.cxx
  qMRML${MODULE_NAME}CurveSettingsWidget.h
  qMRML${MODULE_NAME}CurveSettingsWidget.cxx
  qMRML${MODULE_NAME}OptionsWidgetsFactory.h
  qMRML${MODULE_NAME}OptionsWidgetsFactory.cxx
  qSlicer${MODULE_NAME}PlaceWidget.cxx
  qSlicer${MODULE_NAME}PlaceWidget.h
  qSlicerSimpleMarkupsWidget.cxx
  qSlicerSimpleMarkupsWidget.h
  qSlicerMarkupsMeasurementsWidget.cxx
  qSlicerMarkupsMeasurementsWidget.h
  )

set(${KIT}_MOC_SRCS
  qMRML${MODULE_NAME}DisplayNodeWidget.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.h
  qMRML${MODULE_NAME}InteractionHandleWidget.h
  qMRML${MODULE_NAME}ToolBar.h
  qMRML${MODULE_NAME}ToolBar_p.h
  qMRML${MODULE_NAME}PlaneWidget.h
  qMRML${MODULE_NAME}ROIWidget.h
  qMRML${MODULE_NAME}AbstractOptionsWidget.h
  qMRML${MODULE_NAME}AngleMeasurementsWidget.h
  qMRML${MODULE_NAME}CurveSettingsWidget.h
  qMRML${MODULE_NAME}OptionsWidgetsFactory.h
  qSlicer${MODULE_NAME}PlaceWidget.h
  qSlicerSimpleMarkupsWidget.h
  qSlicerMarkupsMeasurementsWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRML${MODULE_NAME}DisplayNodeWidget.ui
  Resources/UI/qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.ui
  Resources/UI/qMRML${MODULE_NAME}InteractionHandleWidget.ui
  Resources/UI/qMRML${MODULE_NAME}PlaneWidget.ui
  Resources/UI/qMRML${MODULE_NAME}ROIWidget.ui
  Resources/UI/qMRML${MODULE_NAME}AngleMeasurementsWidget.ui
  Resources/UI/qMRML${MODULE_NAME}CurveSettingsWidget.ui
  Resources/UI/qSlicer${MODULE_NAME}PlaceWidget.ui
  Resources/UI/qSlicerSimpleMarkupsWidget.ui
  Resources/UI/qSlicerMarkupsMeasurementsWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicer${MODULE_NAME}SubjectHierarchyPlugins
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
