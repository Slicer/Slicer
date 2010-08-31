#-----------------------------------------------------------------------
#
# SETUP
#
#-----------------------------------------------------------------------
# Module name (usually identique to the directory name)
SET(qt_module_name "Annotation")

SET(qt_module_title "Annotation")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer3_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  )

#-----------------------------------------------------------------------
#
# DISPLAYABLE MANAGERS
#
#-----------------------------------------------------------------------
SET(DisplayableManager_SRCS
  MRMLDisplayableManager/vtkMRMLAnnotationTextThreeDViewDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationFiducialThreeDViewDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationStickyThreeDViewDisplayableManager.cxx  
  MRMLDisplayableManager/vtkMRMLAnnotationAngleThreeDViewDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationSplineThreeDViewDisplayableManager.cxx  
  MRMLDisplayableManager/vtkMRMLAnnotationRulerThreeDViewDisplayableManager.cxx   
  MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationROIThreeDViewDisplayableManager.cxx     
  MRMLDisplayableManager/vtkMRMLAnnotationThreeDViewDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationClickCounter.cxx
  )
  
#-----------------------------------------------------------------------
#
# LOGIC
#
#-----------------------------------------------------------------------
SET(Logic_SRCS
  Logic/vtkSlicerAnnotationModuleLogic.cxx
  Logic/vtkSlicerAnnotationModuleLogic.h
  )

#-----------------------------------------------------------------------
#
# GUI
#
#-----------------------------------------------------------------------
SET(GUI_SRCS
  GUI/qSlicerAnnotationModuleWidget.cxx
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.cxx
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.cxx
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModulePushButton.cxx
  GUI/qSlicerAnnotationModulePushButton.h
  GUI/qSlicerAnnotationModuleScreenShotDialog.cxx
  GUI/qSlicerAnnotationModuleScreenShotDialog.h
  GUI/qSlicerAnnotationModuleImageUtil.cxx
  GUI/qSlicerAnnotationModuleImageUtil.h
  )

#-----------------------------------------------------------------------
#
# DEPRECATED CLASSES (do not get compiled)
#
#-----------------------------------------------------------------------
SET(DEPRECATED_SRCS
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


# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("qSlicer${qt_module_title}ModuleInstantiator"
  DisplayableManagerInstantiator_SRCS
  "${DisplayableManager_SRCS}"
  "${qt_module_export_directive}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${qt_module_title}ModuleExport.h"
  )

#-----------------------------------------------------------------------
#
# ALL SOURCE FILES TOGETHER
#
#-----------------------------------------------------------------------
SET(qt_module_SRCS
  ${DisplayableManager_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  ${Logic_SRCS}
  ${GUI_SRCS}
  #${DEPRECATED_SRCS}
  qSlicerAnnotationModule.cxx
  qSlicerAnnotationModule.h
  )

#-----------------------------------------------------------------------
#
# WIDGETS
#
#-----------------------------------------------------------------------
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

#-----------------------------------------------------------------------
#
# MRML
#
#-----------------------------------------------------------------------
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

#-----------------------------------------------------------------------
#
# MOC HEADERS
#
#-----------------------------------------------------------------------
SET(qt_module_MOC_SRCS
  qSlicerAnnotationModule.h
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModulePushButton.h
  GUI/qSlicerAnnotationModuleScreenShotDialog.h
  )

#-----------------------------------------------------------------------
#
# RESOURCES
#
#-----------------------------------------------------------------------
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerAnnotationModule.ui
  Resources/UI/qSlicerAnnotationModulePropertyDialog.ui
  Resources/UI/qSlicerAnnotationModuleReportDialog.ui
  Resources/UI/qSlicerAnnotationModuleScreenShotDialog.ui
  )
 
# Resources
SET(qt_module_resources
  Resources/qSlicerAnnotationModule.qrc
)

#-----------------------------------------------------------------------
#
# LINK TARGETS
#
#-----------------------------------------------------------------------
# Additional Target libraries
SET(qt_module_target_libraries
  qSlicerBaseQTCore
)

#-----------------------------------------------------------------------
#
# BUILD QTMODULE
#
#-----------------------------------------------------------------------
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

#-----------------------------------------------------------------------
#
# TESTING
#
#-----------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing/Cxx)
endif(BUILD_TESTING)

