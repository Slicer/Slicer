
# Module name (usually identique to the directory name)
SET(qt_module_name "Annotation")

# Build module sub libraries
ADD_SUBDIRECTORY(MRML)
ADD_SUBDIRECTORY(Widgets)

SET(qt_module_export_directive "Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_BINARY_DIR}/MRMLDisplayableManager
  ${CMAKE_CURRENT_SOURCE_DIR}/VTKWidgets
  ${CMAKE_CURRENT_BINARY_DIR}/VTKWidgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

SET(DisplayableManager_SRCS
  MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManagerHelper.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationClickCounter.cxx
  
  MRMLDisplayableManager/vtkMRMLAnnotationTextDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationFiducialDisplayableManager.cxx
  #MRMLDisplayableManager/vtkMRMLAnnotationStickyDisplayableManager.cxx
  #MRMLDisplayableManager/vtkMRMLAnnotationAngleDisplayableManager.cxx
  #MRMLDisplayableManager/vtkMRMLAnnotationSplineDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationRulerDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationBidimensionalDisplayableManager.cxx
  MRMLDisplayableManager/vtkMRMLAnnotationROIDisplayableManager.cxx

  MRMLDisplayableManager/vtkSlicerBoxRepresentation.cxx
  MRMLDisplayableManager/vtkSlicerBoxWidget2.cxx
  )
  
SET(Logic_SRCS
  Logic/vtkSlicerAnnotationModuleLogic.cxx
  Logic/vtkSlicerAnnotationModuleLogic.h
  )
  
SET(VTKWidget_SRCS
  # Ruler VTK widget
  VTKWidgets/vtkAnnotationRulerWidget.cxx
  VTKWidgets/vtkAnnotationRulerWidget.h
  VTKWidgets/vtkAnnotationRulerRepresentation.cxx
  VTKWidgets/vtkAnnotationRulerRepresentation.h
  VTKWidgets/vtkAnnotationRulerRepresentation3D.cxx
  VTKWidgets/vtkAnnotationRulerRepresentation3D.h
  
  # Bidimensional VTK widget
  VTKWidgets/vtkAnnotationBidimensionalWidget.cxx
  VTKWidgets/vtkAnnotationBidimensionalWidget.h
  VTKWidgets/vtkAnnotationBidimensionalRepresentation.cxx
  VTKWidgets/vtkAnnotationBidimensionalRepresentation.h
  
  VTKWidgets/vtkAnnotationGlyphSource2D.cxx
  VTKWidgets/vtkAnnotationGlyphSource2D.h
  )
  
SET(GUI_SRCS
  GUI/qMRMLSceneAnnotationModel.cxx
  GUI/qMRMLSceneAnnotationModel.h
  GUI/qMRMLAnnotationTreeView.cxx
  GUI/qMRMLAnnotationTreeView.h
  GUI/qSlicerAnnotationModuleWidget.cxx
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.cxx
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.cxx
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModuleSnapShotDialog.cxx
  GUI/qSlicerAnnotationModuleSnapShotDialog.h
  )

# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("qSlicer${qt_module_name}ModuleInstantiator"
  DisplayableManagerInstantiator_SRCS
  "${DisplayableManager_SRCS}"
  "${qt_module_export_directive}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${qt_module_name}ModuleExport.h"
  )

# Source files
SET(qt_module_SRCS
  ${DisplayableManager_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  ${Logic_SRCS}
  ${VTKWidget_SRCS}
  ${GUI_SRCS}
  qSlicerAnnotationModule.cxx
  qSlicerAnnotationModule.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerAnnotationModule.h
  GUI/qMRMLSceneAnnotationModel.h
  GUI/qMRMLAnnotationTreeView.h
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModuleSnapShotDialog.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerAnnotationModule.ui
  Resources/UI/qSlicerAnnotationModulePropertyDialog.ui
  Resources/UI/qSlicerAnnotationModuleReportDialog.ui
  Resources/UI/qSlicerAnnotationModuleSnapShotDialog.ui
  )
 
# Resources
SET(qt_module_resources
  Resources/qSlicerAnnotationModule.qrc
  )

# Additional Target libraries
SET(qt_module_target_libraries
  qSlicerBaseQTCore
  qMRMLWidgets
  vtkSlicerAnnotationModuleMRML
  qSlicerAnnotationModuleWidgets
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
  ADD_SUBDIRECTORY(Testing/Cxx)
endif()

