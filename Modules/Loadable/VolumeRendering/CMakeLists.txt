
#-----------------------------------------------------------------------------
set(MODULE_NAME "VolumeRendering") # Do not use 'project()'
set(MODULE_TITLE "Volume Rendering")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(VolumeRenderingReplacements)
add_subdirectory(Logic)
add_subdirectory(MRMLDisplayableManager)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${qSlicerAnnotationsModuleWidgets_SOURCE_DIR}
  ${qSlicerAnnotationsModuleWidgets_BINARY_DIR}
  )

set(MODULE_SRCS
  qSlicerVolumeRenderingIO.cxx
  qSlicerVolumeRenderingIO.h
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingSettingsPanel.cxx
  qSlicerVolumeRenderingSettingsPanel.h
  )

set(MODULE_MOC_SRCS
  qSlicerVolumeRenderingIO.h
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingSettingsPanel.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingSettingsPanel.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager
  qSlicer${MODULE_NAME}ModuleWidgets

  vtkSlicerAnnotationsModuleMRML
  qSlicerAnnotationsModuleWidgets
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
