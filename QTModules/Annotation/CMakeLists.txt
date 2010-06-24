cmake_minimum_required(VERSION 2.8)

# Module name (usually identique to the directory name)
SET(qt_module_name "Annotation")

SET(qt_module_title "Annotation")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT")

# Additional directories to include
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  #${CMAKE_CURRENT_SOURCE_DIR}/../../Modules/Measurements/
  #${CMAKE_CURRENT_BINARY_DIR}/../../Modules/Measurements/
  )
#   ${CMAKE_CURRENT_BINARY_DIR}/Widgets
# MESSAGE(STATUS  "=========>What ${CMAKE_CURRENT_BINARY_DIR}/../../Modules/Measurements/")  

# Source files
SET(qt_module_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationDisplayNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationDisplayNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationTextDisplayNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationTextDisplayNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationControlPointsNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationControlPointsNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationControlPointsStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationControlPointsStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationPointDisplayNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationPointDisplayNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLinesNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLinesNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLinesStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLinesStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLineDisplayNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationLineDisplayNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationAngleNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationAngleNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationAngleStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationAngleStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationRulerNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationRulerNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationRulerStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationRulerStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationFiducialNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationFiducialNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationFiducialsStorageNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationFiducialsStorageNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationStickyNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationStickyNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationTextNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationTextNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationROINode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationROINode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationSplineNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationSplineNode.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationBidimensionalNode.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML/vtkMRMLAnnotationBidimensionalNode.h
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
  vtkSlicerAnnotationRulerManager.cxx
  vtkSlicerAnnotationRulerManager.h
  vtkSlicerAnnotationAngleManager.cxx
  vtkSlicerAnnotationAngleManager.h
  vtkSlicerAnnotationFiducialManager.cxx
  vtkSlicerAnnotationFiducialManager.h
  vtkSlicerAnnotationROIManager.cxx
  vtkSlicerAnnotationROIManager.h
  vtkSlicerAnnotationSplineManager.cxx
  vtkSlicerAnnotationSplineManager.h
  vtkSlicerAnnotationBidimensionalManager.cxx
  vtkSlicerAnnotationBidimensionalManager.h
  qSlicerAnnotationTextManager.cxx
  qSlicerAnnotationTextManager.h
  # ${CMAKE_CURRENT_SOURCE_DIR}/Widgets/vtkAnnotationAngleWidget.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/Widgets/vtkAnnotationAngleWidget.h
  # ${CMAKE_CURRENT_SOURCE_DIR}/Widgets/vtkAnnotationAngleWidgetClass.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/Widgets/vtkAnnotationAngleWidgetClass.h
  )

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
  #SlicerTractographyDisplay
  #SlicerTractographyFiducialSeeding
  qSlicerBaseQTCore
  #Measurements
  #
  
  )

# not needed anymore - DH
#file(GLOB headers
#  "${CMAKE_CURRENT_SOURCE_DIR}/../Modules/Measurement/*.h"
#®)

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
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

