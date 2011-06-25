
# Module name (usually identique to the directory name)
set(qt_module_name "VolumeRendering")
set(qt_module_title "Volume Rendering")

# Build module sub libraries
add_subdirectory(MRML)
add_subdirectory(Widgets)
add_subdirectory(VolumeRenderingReplacements)
add_subdirectory(Logic)

set(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
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
  ${qSlicerAnnotationModuleWidgets_SOURCE_DIR}
  ${qSlicerAnnotationModuleWidgets_BINARY_DIR}
  )

# Displayable manager sources
set(qt_module_displayable_manager_SRCS
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.h
  )

# Create the instantiator for DisplayableManager classes.
set(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${qt_module_name}Instantiator"
  qt_module_displayable_manager_instantiator_SRCS
  "${qt_module_displayable_manager_SRCS}"
  "${qt_module_export_directive}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${qt_module_name}ModuleExport.h"
  )


# Source files
set(qt_module_SRCS
  ${qt_module_displayable_manager_SRCS}
  ${qt_module_displayable_manager_instantiator_SRCS}
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.cxx
  qSlicerVolumeRenderingModuleWidget.h
  )

# Headers that should run through moc
set(qt_module_MOC_SRCS
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.h
  )

# UI files
set(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingModule.ui
  )

# Additional Target libraries
set(qt_module_target_libraries
  vtkSlicerVolumeRenderingModuleLogic
  vtkSlicerVolumeRenderingModuleMRML
  VolumeRenderingReplacements
  vtkSlicerAnnotationModuleMRML
  qSlicerAnnotationModuleWidgets
  qSlicerVolumeRenderingModuleWidgets
  )

# Resources
set(qt_module_resources
  Resources/qSlicerVolumeRenderingModule.qrc
  )

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
