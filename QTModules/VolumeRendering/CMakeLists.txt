
# Module name (usually identique to the directory name)
SET(qt_module_name "VolumeRendering")

# Build module sub libraries
ADD_SUBDIRECTORY(MRML)
ADD_SUBDIRECTORY(VolumeRenderingReplacements)
ADD_SUBDIRECTORY(Logic)

SET(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_BINARY_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/VolumeRenderingReplacements
  ${CMAKE_CURRENT_BINARY_DIR}/VolumeRenderingReplacements
  ${qSlicerAnnotationModuleWidgets_SOURCE_DIR}
  ${qSlicerAnnotationModuleWidgets_BINARY_DIR}
  )

# Displayable manager sources
SET(qt_module_displayable_manager_SRCS
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLVolumeRenderingDisplayableManager.h
  )

# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${qt_module_name}Instantiator"
  qt_module_displayable_manager_instantiator_SRCS
  "${qt_module_displayable_manager_SRCS}"
  "${qt_module_export_directive}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${qt_module_name}ModuleExport.h"
  )


# Source files
SET(qt_module_SRCS
  ${qt_module_displayable_manager_SRCS}
  ${qt_module_displayable_manager_instantiator_SRCS}
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.cxx
  qSlicerVolumeRenderingModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingModule.ui
  )

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerVolumeRenderingModuleLogic
  vtkSlicerVolumeRenderingModuleMRML
  VolumeRenderingReplacements
  vtkSlicerAnnotationModuleMRML
  qSlicerAnnotationModuleWidgets
  )

# Resources
SET(qt_module_resources
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
  ADD_SUBDIRECTORY(Testing)
endif()
