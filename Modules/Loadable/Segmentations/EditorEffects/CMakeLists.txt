project(qSlicer${MODULE_NAME}EditorEffects)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_${MODULE_NAME_UPPER}_EFFECTS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleMRMLDisplayableManager_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleLogic_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${qMRMLWidgets_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerSegmentEditorEffectFactory.cxx
  qSlicerSegmentEditorEffectFactory.h
  qSlicerSegmentEditorAbstractEffect.cxx
  qSlicerSegmentEditorAbstractEffect.h
  qSlicerSegmentEditorAbstractEffect_p.h
  qSlicerSegmentEditorAbstractLabelEffect.cxx
  qSlicerSegmentEditorAbstractLabelEffect.h
  qSlicerSegmentEditorAbstractLabelEffect_p.h
  qSlicerSegmentEditorPaintEffect.cxx
  qSlicerSegmentEditorPaintEffect.h
  qSlicerSegmentEditorPaintEffect_p.h
  qSlicerSegmentEditorEraseEffect.cxx
  qSlicerSegmentEditorEraseEffect.h
  qSlicerSegmentEditorScissorsEffect.cxx
  qSlicerSegmentEditorScissorsEffect.h
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND ${KIT}_SRCS
    qSlicerSegmentEditorScriptedEffect.cxx
    qSlicerSegmentEditorScriptedEffect.h
    qSlicerSegmentEditorScriptedLabelEffect.cxx
    qSlicerSegmentEditorScriptedLabelEffect.h
    )
endif()

set(${KIT}_MOC_SRCS
  qSlicerSegmentEditorEffectFactory.h
  qSlicerSegmentEditorAbstractEffect.h
  qSlicerSegmentEditorAbstractEffect_p.h
  qSlicerSegmentEditorAbstractLabelEffect.h
  qSlicerSegmentEditorAbstractLabelEffect_p.h
  qSlicerSegmentEditorPaintEffect.h
  qSlicerSegmentEditorPaintEffect_p.h
  qSlicerSegmentEditorEraseEffect.h
  qSlicerSegmentEditorScissorsEffect.h
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND ${KIT}_MOC_SRCS
    qSlicerSegmentEditorScriptedEffect.h
    qSlicerSegmentEditorScriptedLabelEffect.h
    )
endif()

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

#-----------------------------------------------------------------------------
set(${KIT}_TARGET_LIBRARIES
  vtkSlicerSegmentationsModuleMRML
  vtkSlicerSegmentationsModuleMRMLDisplayableManager
  vtkSlicerSegmentationsModuleLogic
  qMRMLWidgets
  MRMLLogic
  MRMLCore
  qSlicerBaseQTGUI # required for access to qSlicerApplication singletons
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleQtLibrary(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  FOLDER "Module-${MODULE_NAME}"
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
if(Slicer_USE_PYTHONQT)
  add_subdirectory(Python)
endif()
