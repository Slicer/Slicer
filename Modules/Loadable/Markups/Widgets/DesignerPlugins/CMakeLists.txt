
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.h
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h
  qMRML${MODULE_NAME}DisplayNodeWidgetPlugin.cxx
  qMRML${MODULE_NAME}DisplayNodeWidgetPlugin.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidgetPlugin.cxx
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidgetPlugin.h
  qMRML${MODULE_NAME}InteractionHandleWidgetPlugin.cxx
  qMRML${MODULE_NAME}InteractionHandleWidgetPlugin.h
  qMRML${MODULE_NAME}ROIWidgetPlugin.cxx
  qMRML${MODULE_NAME}ROIWidgetPlugin.h
  qSlicer${MODULE_NAME}PlaceWidgetPlugin.cxx
  qSlicer${MODULE_NAME}PlaceWidgetPlugin.h
  qSlicerSimpleMarkupsWidgetPlugin.cxx
  qSlicerSimpleMarkupsWidgetPlugin.h
  qSlicerMarkupsMeasurementsWidgetPlugin.cxx
  qSlicerMarkupsMeasurementsWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h
  qMRML${MODULE_NAME}DisplayNodeWidgetPlugin.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidgetPlugin.h
  qMRML${MODULE_NAME}InteractionHandleWidgetPlugin.h
  qMRML${MODULE_NAME}ROIWidgetPlugin.h
  qSlicerMarkupsPlaceWidgetPlugin.h
  qSlicerSimpleMarkupsWidgetPlugin.h
  qSlicerMarkupsMeasurementsWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicer${MODULE_NAME}ModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Module-${MODULE_NAME}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
