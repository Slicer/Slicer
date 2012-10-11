
#-----------------------------------------------------------------------------
set(MODULE_NAME "Annotations")  # Do not use 'project()'
set(MODULE_TITLE "${MODULE_NAME}")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
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

set(DisplayableManager_SRCS
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
  MRMLDisplayableManager/vtkMRMLAnnotationROIDisplayableManager.h
  MRMLDisplayableManager/vtkMRMLAnnotationROIDisplayableManager.cxx

  )

set(VTKWidget_SRCS
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

  VTKWidgets/vtkAnnotationROIRepresentation.h
  VTKWidgets/vtkAnnotationROIRepresentation.cxx
  VTKWidgets/vtkAnnotationROIRepresentation2D.h
  VTKWidgets/vtkAnnotationROIRepresentation2D.cxx
  VTKWidgets/vtkAnnotationROIWidget.h
  VTKWidgets/vtkAnnotationROIWidget.cxx
  VTKWidgets/vtkAnnotationROIWidget2D.h
  VTKWidgets/vtkAnnotationROIWidget2D.cxx
  )

set(GUI_SRCS
  GUI/qSlicerAnnotationModuleWidget.cxx
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.cxx
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.cxx
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModuleSnapShotDialog.cxx
  GUI/qSlicerAnnotationModuleSnapShotDialog.h
  )

set(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("qSlicer${MODULE_NAME}ModuleInstantiator"
  DisplayableManagerInstantiator_SRCS
  "${DisplayableManager_SRCS}"
  "${MODULE_EXPORT_DIRECTIVE}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "qSlicer${MODULE_NAME}ModuleExport.h"
  )

set(MODULE_SRCS
  ${DisplayableManager_SRCS}
  ${DisplayableManagerInstantiator_SRCS}
  ${VTKWidget_SRCS}
  ${GUI_SRCS}
  qSlicerAnnotationsIO.cxx
  qSlicerAnnotationsIO.h
  qSlicerAnnotationsIOOptionsWidget.cxx
  qSlicerAnnotationsIOOptionsWidget.h
  qSlicerAnnotationsModule.cxx
  qSlicerAnnotationsModule.h
  qSlicerFiducialsIO.cxx
  qSlicerFiducialsIO.h
  )

set(MODULE_MOC_SRCS
  qSlicerAnnotationsIO.h
  qSlicerAnnotationsIOOptionsWidget.h
  qSlicerAnnotationsModule.h
  qSlicerFiducialsIO.h
  GUI/qSlicerAnnotationModuleWidget.h
  GUI/qSlicerAnnotationModulePropertyDialog.h
  GUI/qSlicerAnnotationModuleReportDialog.h
  GUI/qSlicerAnnotationModuleSnapShotDialog.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerAnnotationModule.ui
  Resources/UI/qSlicerAnnotationModuleIOOptionsWidget.ui
  Resources/UI/qSlicerAnnotationModulePropertyDialog.ui
  Resources/UI/qSlicerAnnotationModuleReportDialog.ui
  )

set(MODULE_RESOURCES
  Resources/qSlicerAnnotationModule.qrc
  )

set(MODULE_TARGET_LIBRARIES
  qSlicerBaseQTCore
  qMRMLWidgets
  vtkSlicerAnnotationsModuleMRML
  vtkSlicerAnnotationsModuleLogic
  qSlicerAnnotationsModuleWidgets
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
  add_subdirectory(Testing/Cxx)
  if(Slicer_USE_PYTHONQT)
    add_subdirectory(Testing/Python)
  endif()
endif()

