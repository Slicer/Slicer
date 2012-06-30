project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  qSlicerDiffusionTensorVolumeDisplayWidget.cxx
  qSlicerDiffusionTensorVolumeDisplayWidget.h
  qSlicerDiffusionWeightedVolumeDisplayWidget.cxx
  qSlicerDiffusionWeightedVolumeDisplayWidget.h
  qSlicerLabelMapVolumeDisplayWidget.cxx
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.cxx
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumeDisplayWidget.cxx
  qSlicerVolumeDisplayWidget.h
  qSlicerDTISliceDisplayWidget.cxx
  qSlicerDTISliceDisplayWidget.h
  )

set(${KIT}_MOC_SRCS
  qSlicerDiffusionTensorVolumeDisplayWidget.h
  qSlicerDiffusionWeightedVolumeDisplayWidget.h
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumeDisplayWidget.h
  qSlicerDTISliceDisplayWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerDiffusionTensorVolumeDisplayWidget.ui
  Resources/UI/qSlicerDiffusionWeightedVolumeDisplayWidget.ui
  Resources/UI/qSlicerDTISliceDisplayWidget.ui
  Resources/UI/qSlicerLabelMapVolumeDisplayWidget.ui
  Resources/UI/qSlicerScalarVolumeDisplayWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/qSlicer${MODULE_NAME}WidgetsModule.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
