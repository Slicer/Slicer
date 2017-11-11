project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  ${vtkSlicerDICOMLibModuleLogic_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerDICOMExportDialog.cxx
  qSlicerDICOMExportDialog.h
  qSlicerDICOMLoadable.cxx
  qSlicerDICOMLoadable.h
  qSlicerDICOMExportable.cxx
  qSlicerDICOMExportable.h
  qSlicerDICOMTagEditorWidget.cxx
  qSlicerDICOMTagEditorWidget.h
  )

set(${KIT}_MOC_SRCS
  qSlicerDICOMExportDialog.h
  qSlicerDICOMLoadable.h
  qSlicerDICOMExportable.h
  qSlicerDICOMTagEditorWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerDICOMExportDialog.ui
  )

set(${KIT}_RESOURCES
  #Resources/${KIT}.qrc
  )

#-----------------------------------------------------------------------------
set(${KIT}_TARGET_LIBRARIES
  qSlicerSubjectHierarchyModuleWidgets
  vtkSlicerDICOMLibModuleLogic
  MRMLCore
  MRMLLogic
  ${QT_LIBRARIES}
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
  #add_subdirectory(Testing)
endif()
