project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSubjectHierarchyModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${qSlicerSubjectHierarchyModulePlugins_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLSubjectHierarchyTreeView.cxx
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSceneSubjectHierarchyModel.cxx
  qMRMLSceneSubjectHierarchyModel.h
  qMRMLSceneSubjectHierarchyModel_p.h
  qMRMLSortFilterSubjectHierarchyProxyModel.cxx
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  qMRMLPotentialSubjectHierarchyListView.cxx
  qMRMLPotentialSubjectHierarchyListView.h
  qMRMLScenePotentialSubjectHierarchyModel.cxx
  qMRMLScenePotentialSubjectHierarchyModel.h
  qMRMLScenePotentialSubjectHierarchyModel_p.h
  qMRMLSortFilterPotentialSubjectHierarchyProxyModel.cxx
  qMRMLSortFilterPotentialSubjectHierarchyProxyModel.h
  qMRMLTransformItemDelegate.cxx
  qMRMLTransformItemDelegate.h
  )

set(${KIT}_MOC_SRCS
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSceneSubjectHierarchyModel.h
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  qMRMLPotentialSubjectHierarchyListView.h
  qMRMLScenePotentialSubjectHierarchyModel.h
  qMRMLSortFilterPotentialSubjectHierarchyProxyModel.h
  qMRMLTransformItemDelegate.h
  )

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  # Resources/${KIT}.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  qMRMLWidgets
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicer${MODULE_NAME}ModulePlugins
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
  #add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
add_subdirectory(DesignerPlugins)
