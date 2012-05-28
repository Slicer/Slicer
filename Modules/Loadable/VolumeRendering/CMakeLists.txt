
#-----------------------------------------------------------------------------
set(MODULE_NAME "VolumeRendering") # Do not use 'project()'
set(MODULE_TITLE "Volume Rendering")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Widgets)
add_subdirectory(VolumeRenderingReplacements)
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_BINARY_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/VolumeRenderingReplacements
  ${CMAKE_CURRENT_BINARY_DIR}/VolumeRenderingReplacements
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${qSlicerAnnotationsModuleWidgets_SOURCE_DIR}
  ${qSlicerAnnotationsModuleWidgets_BINARY_DIR}
  )

set(qt_module_displayable_manager_SRCS
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.h
  )

set(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${MODULE_NAME}Instantiator"
  qt_module_displayable_manager_instantiator_SRCS
  "${qt_module_displayable_manager_SRCS}"
  "${MODULE_EXPORT_DIRECTIVE}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${MODULE_NAME}ModuleExport.h"
  )

set(MODULE_SRCS
  ${qt_module_displayable_manager_SRCS}
  ${qt_module_displayable_manager_instantiator_SRCS}
  qSlicerVolumeRenderingIO.cxx
  qSlicerVolumeRenderingIO.h
  qSlicerVolumeRenderingSettingsPanel.cxx
  qSlicerVolumeRenderingSettingsPanel.h
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.cxx
  qSlicerVolumeRenderingModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerVolumeRenderingIO.h
  qSlicerVolumeRenderingSettingsPanel.h
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingSettingsPanel.ui
  Resources/UI/qSlicerVolumeRenderingModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerVolumeRenderingModuleLogic
  vtkSlicerVolumeRenderingModuleMRML
  VolumeRenderingReplacements
  vtkSlicerAnnotationsModuleMRML
  qSlicerAnnotationsModuleWidgets
  qSlicerVolumeRenderingModuleWidgets
  )

set(MODULE_RESOURCES
  Resources/qSlicerVolumeRenderingModule.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
