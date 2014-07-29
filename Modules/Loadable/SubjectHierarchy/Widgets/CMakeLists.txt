project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSubjectHierarchyModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLSubjectHierarchyTreeView.cxx
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSceneSubjectHierarchyModel.cxx
  qMRMLSceneSubjectHierarchyModel.h
  qMRMLSceneSubjectHierarchyModel_p.h
  qMRMLSortFilterSubjectHierarchyProxyModel.cxx
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  qMRMLTransformItemDelegate.cxx
  qMRMLTransformItemDelegate.h
  qSlicerSubjectHierarchyAbstractPlugin.cxx
  qSlicerSubjectHierarchyAbstractPlugin.h
  qSlicerSubjectHierarchyPluginHandler.cxx
  qSlicerSubjectHierarchyPluginHandler.h
  qSlicerSubjectHierarchyDefaultPlugin.cxx
  qSlicerSubjectHierarchyDefaultPlugin.h
  qSlicerSubjectHierarchyCloneNodePlugin.cxx
  qSlicerSubjectHierarchyCloneNodePlugin.h
  qSlicerSubjectHierarchyDICOMPlugin.cxx
  qSlicerSubjectHierarchyDICOMPlugin.h
  qSlicerSubjectHierarchyChartsPlugin.cxx
  qSlicerSubjectHierarchyChartsPlugin.h
  qSlicerSubjectHierarchyParseLocalDataPlugin.cxx
  qSlicerSubjectHierarchyParseLocalDataPlugin.h
  qSlicerSubjectHierarchyRegisterPlugin.cxx
  qSlicerSubjectHierarchyRegisterPlugin.h
  )

set(${KIT}_MOC_SRCS
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSceneSubjectHierarchyModel.h
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  qMRMLTransformItemDelegate.h
  qSlicerSubjectHierarchyAbstractPlugin.h
  qSlicerSubjectHierarchyPluginHandler.h
  qSlicerSubjectHierarchyDefaultPlugin.h
  qSlicerSubjectHierarchyCloneNodePlugin.h
  qSlicerSubjectHierarchyDICOMPlugin.h
  qSlicerSubjectHierarchyChartsPlugin.h
  qSlicerSubjectHierarchyParseLocalDataPlugin.h
  qSlicerSubjectHierarchyRegisterPlugin.h
  )

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

#-----------------------------------------------------------------------------
set_source_files_properties( qSlicerSubjectHierarchyAbstractPlugin.h WRAP_EXCLUDE )

SET (${KIT}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE)

set(${KIT}_TARGET_LIBRARIES
  qMRMLWidgets
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  MRMLCore
  MRMLLogic
  MRMLCLI
  qMRMLWidgets
  ${QT_LIBRARIES}
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
