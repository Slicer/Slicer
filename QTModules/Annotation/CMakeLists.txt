
# Module name (usually identique to the directory name)
SET(qt_module_name "Annotation")

SET(qt_module_title "Annotation")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT")

# Additional directories to include
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  )

# DisplayableManager classes
SET(DisplayableManager_SRCS
  vtkMRMLAnnotationTextDisplayableManager.cxx
  vtkMRMLAnnotationFiducialDisplayableManager.cxx
  )

# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("qSlicer${qt_module_title}ModuleInstantiator"
  DisplayableManagerInstantiator_SRCS
  "${DisplayableManager_SRCS}"
  "${qt_module_export_directive}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${qt_module_title}ModuleExport.h")

# Source files
SET(qt_module_SRCS
  ${DisplayableManager_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  qSlicerAnnotationModule.cxx
  qSlicerAnnotationModule.h
  vtkSlicerAnnotationModuleLogic.cxx
  vtkSlicerAnnotationModuleLogic.h
  qSlicerAnnotationModuleWidget.cxx
  qSlicerAnnotationModuleWidget.h
  qSlicerAnnotationModuleAnnotationPropertyDialog.cxx
  qSlicerAnnotationModuleAnnotationPropertyDialog.h
  qSlicerAnnotationModuleReportDialog.cxx
  qSlicerAnnotationModuleReportDialog.h
  qSlicerAnnotationModulePushButton.cxx
  qSlicerAnnotationModulePushButton.h
  qSlicerAnnotationModuleScreenShotDialog.cxx
  qSlicerAnnotationModuleScreenShotDialog.h
  qSlicerAnnotationModuleImageUtil.cxx
  qSlicerAnnotationModuleImageUtil.h
  vtkMRMLAnnotationDisplayableManager.cxx
  vtkMRMLAnnotationDisplayableManager.h
  vtkSlicerAnnotationRulerManager.cxx
  vtkSlicerAnnotationRulerManager.h
  vtkSlicerAnnotationAngleManager.cxx
  vtkSlicerAnnotationAngleManager.h
  vtkSlicerAnnotationROIManager.cxx
  vtkSlicerAnnotationROIManager.h
  vtkSlicerAnnotationSplineManager.cxx
  vtkSlicerAnnotationSplineManager.h
  vtkSlicerAnnotationBidimensionalManager.cxx
  vtkSlicerAnnotationBidimensionalManager.h
  )

SET(MODULE_WIDGETS_SRCS
  #vtkAnnotationAngleWidget.cxx
  #vtkAnnotationAngleWidget.h
  #vtkAnnotationAngleWidgetClass.cxx
  #vtkAnnotationAngleWidgetClass.h
  )

# Expand WIDGETS sources filename and append to module sources
FOREACH(file ${MODULE_WIDGETS_SRCS})
  LIST(APPEND qt_module_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/Widgets/${file})
ENDFOREACH()

SET(MODULE_MRML_SRCS
  vtkMRMLAnnotationNode.cxx
  vtkMRMLAnnotationNode.h
  vtkMRMLAnnotationStorageNode.cxx
  vtkMRMLAnnotationStorageNode.h
  vtkMRMLAnnotationDisplayNode.cxx
  vtkMRMLAnnotationDisplayNode.h
  vtkMRMLAnnotationTextDisplayNode.cxx
  vtkMRMLAnnotationTextDisplayNode.h
  vtkMRMLAnnotationControlPointsNode.cxx
  vtkMRMLAnnotationControlPointsNode.h
  vtkMRMLAnnotationControlPointsStorageNode.cxx
  vtkMRMLAnnotationControlPointsStorageNode.h
  vtkMRMLAnnotationPointDisplayNode.cxx
  vtkMRMLAnnotationPointDisplayNode.h
  vtkMRMLAnnotationLinesNode.cxx
  vtkMRMLAnnotationLinesNode.h
  vtkMRMLAnnotationLinesStorageNode.cxx
  vtkMRMLAnnotationLinesStorageNode.h
  vtkMRMLAnnotationLineDisplayNode.cxx
  vtkMRMLAnnotationLineDisplayNode.h
  vtkMRMLAnnotationAngleNode.cxx
  vtkMRMLAnnotationAngleNode.h
  vtkMRMLAnnotationAngleStorageNode.cxx
  vtkMRMLAnnotationAngleStorageNode.h
  vtkMRMLAnnotationRulerNode.cxx
  vtkMRMLAnnotationRulerNode.h
  vtkMRMLAnnotationRulerStorageNode.cxx
  vtkMRMLAnnotationRulerStorageNode.h
  vtkMRMLAnnotationFiducialNode.cxx
  vtkMRMLAnnotationFiducialNode.h
  vtkMRMLAnnotationFiducialsStorageNode.cxx
  vtkMRMLAnnotationFiducialsStorageNode.h
  vtkMRMLAnnotationStickyNode.cxx
  vtkMRMLAnnotationStickyNode.h
  vtkMRMLAnnotationTextNode.cxx
  vtkMRMLAnnotationTextNode.h
  vtkMRMLAnnotationROINode.cxx
  vtkMRMLAnnotationROINode.h
  vtkMRMLAnnotationSplineNode.cxx
  vtkMRMLAnnotationSplineNode.h
  vtkMRMLAnnotationBidimensionalNode.cxx
  vtkMRMLAnnotationBidimensionalNode.h
  )

# Expand MRML sources filename and append to module sources
FOREACH(file ${MODULE_MRML_SRCS})
  LIST(APPEND qt_module_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/MRML/${file})
ENDFOREACH()

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerAnnotationModule.h
  qSlicerAnnotationModuleWidget.h
  qSlicerAnnotationModuleAnnotationPropertyDialog.h
  qSlicerAnnotationModuleReportDialog.h
  qSlicerAnnotationModulePushButton.h
  qSlicerAnnotationModuleScreenShotDialog.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerAnnotationModule.ui
  Resources/UI/qSlicerAnnotationModuleAnnotationPropertyDialog.ui
  Resources/UI/qSlicerAnnotationModuleReportDialog.ui
  Resources/UI/qSlicerAnnotationModuleScreenShotDialog.ui
)
 
# Resources
SET(qt_module_resources
  Resources/qSlicerAnnotationModule.qrc
)

# Additional Target libraries
SET(qt_module_target_libraries
  #${KWWidgets_LIBRARIES}
  #${ITK_LIBRARIES}
  #CommandLineModule
  qSlicerBaseQTCore
  #Measurements
  )

Slicer3_build_qtmodule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing/Cxx)
endif(BUILD_TESTING)

