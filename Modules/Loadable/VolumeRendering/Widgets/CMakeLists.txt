project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
  )

set(${KIT}_SRCS
  qSlicerCPURayCast${MODULE_NAME}PropertiesWidget.cxx
  qSlicerCPURayCast${MODULE_NAME}PropertiesWidget.h
  qSlicerGPURayCast${MODULE_NAME}PropertiesWidget.cxx
  qSlicerGPURayCast${MODULE_NAME}PropertiesWidget.h
  qSlicerMulti${MODULE_NAME}PropertiesWidget.cxx
  qSlicerMulti${MODULE_NAME}PropertiesWidget.h
  qSlicerPresetComboBox.cxx
  qSlicerPresetComboBox.h
  qSlicerPresetComboBox_p.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicer${MODULE_NAME}PropertiesWidget.cxx
  qSlicer${MODULE_NAME}PropertiesWidget.h
  qSlicer${MODULE_NAME}PresetComboBox.cxx
  qSlicer${MODULE_NAME}PresetComboBox.h
  qMRMLVolumePropertyNodeWidget.cxx
  qMRMLVolumePropertyNodeWidget.h
  qSlicerGPUMemoryComboBox.cxx
  qSlicerGPUMemoryComboBox.h
  )

set(${KIT}_MOC_SRCS
  qSlicerCPURayCast${MODULE_NAME}PropertiesWidget.h
  qSlicerGPURayCast${MODULE_NAME}PropertiesWidget.h
  qSlicerMulti${MODULE_NAME}PropertiesWidget.h
  qSlicerPresetComboBox.h
  qSlicerPresetComboBox_p.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicer${MODULE_NAME}PropertiesWidget.h
  qSlicer${MODULE_NAME}PresetComboBox.h
  qMRMLVolumePropertyNodeWidget.h
  qSlicerGPUMemoryComboBox.h
  )

set(${KIT}_UI_SRCS
  ../Resources/UI/qMRMLVolumePropertyNodeWidget.ui
  ../Resources/UI/qSlicerCPURayCast${MODULE_NAME}PropertiesWidget.ui
  ../Resources/UI/qSlicerGPURayCast${MODULE_NAME}PropertiesWidget.ui
  ../Resources/UI/qSlicerMulti${MODULE_NAME}PropertiesWidget.ui
  ../Resources/UI/qSlicer${MODULE_NAME}ModuleWidget.ui
  ../Resources/UI/qSlicer${MODULE_NAME}PresetComboBox.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  qSlicerAnnotationsModuleWidgets
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
