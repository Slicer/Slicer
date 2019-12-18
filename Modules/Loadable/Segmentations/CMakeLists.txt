#-----------------------------------------------------------------------------
set(MODULE_NAME "Segmentations")
set(MODULE_TITLE ${MODULE_NAME})

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(MRMLDM)
add_subdirectory(EditorEffects)
add_subdirectory(Widgets)
add_subdirectory(SubjectHierarchyPlugins)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

set(MODULE_INCLUDE_DIRECTORIES
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleMRMLDisplayableManager_INCLUDE_DIRS}
  ${qSlicerSegmentationsModuleWidgets_INCLUDE_DIRS}
  ${qSlicerSegmentationsEditorEffects_INCLUDE_DIRS}
  ${qSlicerSegmentationsSubjectHierarchyPlugins_INCLUDE_DIRS}
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  ${qSlicerTerminologiesModuleWidgets_INCLUDE_DIRS}
  ${vtkSlicerTerminologiesModuleLogic_INCLUDE_DIRS}
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}IOOptionsWidget.cxx
  qSlicer${MODULE_NAME}IOOptionsWidget.h
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicer${MODULE_NAME}NodeWriter.cxx
  qSlicer${MODULE_NAME}NodeWriter.h
  qSlicer${MODULE_NAME}NodeWriterOptionsWidget.cxx
  qSlicer${MODULE_NAME}NodeWriterOptionsWidget.h
  qSlicer${MODULE_NAME}Reader.cxx
  qSlicer${MODULE_NAME}Reader.h
  qSlicer${MODULE_NAME}SettingsPanel.cxx
  qSlicer${MODULE_NAME}SettingsPanel.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}IOOptionsWidget.h
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicer${MODULE_NAME}NodeWriter.h
  qSlicer${MODULE_NAME}NodeWriterOptionsWidget.h
  qSlicer${MODULE_NAME}Reader.h
  qSlicer${MODULE_NAME}SettingsPanel.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}IOOptionsWidget.ui
  Resources/UI/qSlicer${MODULE_NAME}Module.ui
  Resources/UI/qSlicer${MODULE_NAME}SettingsPanel.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager
  qSlicer${MODULE_NAME}ModuleWidgets
  qSlicer${MODULE_NAME}SubjectHierarchyPlugins
  qSlicer${MODULE_NAME}EditorEffects
  qSlicerSubjectHierarchyModuleWidgets
  qSlicerTerminologiesModuleWidgets
  vtkSlicerTerminologiesModuleLogic
  qSlicerBaseQTGUI
  )

set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildLoadableModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
