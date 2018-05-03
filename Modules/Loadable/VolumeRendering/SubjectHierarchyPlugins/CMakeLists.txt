project(qSlicer${MODULE_NAME}SubjectHierarchyPlugins)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_${MODULE_NAME_UPPER}_SUBJECT_HIERARCHY_PLUGINS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerVolumeRenderingModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerVolumeRenderingModuleLogic_INCLUDE_DIRS}
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${qMRMLWidgets_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerSubjectHierarchyVolumeRenderingPlugin.cxx
  qSlicerSubjectHierarchyVolumeRenderingPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicerSubjectHierarchyVolumeRenderingPlugin.h
  )

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicerSubjectHierarchyModuleWidgets
  vtkSlicerSubjectHierarchyModuleLogic
  qMRMLWidgets
  MRMLLogic
  MRMLCore
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
