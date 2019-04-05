project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  )

set(${KIT}_SRCS
  qMRML${MODULE_NAME}DisplayNodeWidget.cxx
  qMRML${MODULE_NAME}DisplayNodeWidget.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.cxx
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.h
  qSlicerMarkupsPlaceWidget.cxx
  qSlicerMarkupsPlaceWidget.h
  qSlicerSimpleMarkupsWidget.cxx
  qSlicerSimpleMarkupsWidget.h
  )

set(${KIT}_MOC_SRCS
  qMRML${MODULE_NAME}DisplayNodeWidget.h
  qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.h
  qSlicerMarkupsPlaceWidget.h
  qSlicerSimpleMarkupsWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRML${MODULE_NAME}DisplayNodeWidget.ui
  Resources/UI/qMRML${MODULE_NAME}FiducialProjectionPropertyWidget.ui
  Resources/UI/qSlicerMarkupsPlaceWidget.ui
  Resources/UI/qSlicerSimpleMarkupsWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
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
