project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${qSlicerTerminologiesModuleWidgets_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND ${KIT}_INCLUDE_DIRECTORIES
    ${MRMLCLI_INCLUDE_DIRS}
  )
endif()

set(${KIT}_SRCS
  qMRMLSubjectHierarchyModel.cxx
  qMRMLSubjectHierarchyModel.h
  qMRMLSubjectHierarchyModel_p.h
  qMRMLSortFilterSubjectHierarchyProxyModel.cxx
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  qMRMLSubjectHierarchyTreeView.cxx
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSubjectHierarchyComboBox.cxx
  qMRMLSubjectHierarchyComboBox.h
  qSlicerSubjectHierarchyPluginLogic.cxx
  qSlicerSubjectHierarchyPluginLogic.h
  qSlicerSubjectHierarchyAbstractPlugin.cxx
  qSlicerSubjectHierarchyAbstractPlugin.h
  qSlicerSubjectHierarchyPluginHandler.cxx
  qSlicerSubjectHierarchyPluginHandler.h
  qSlicerSubjectHierarchyDefaultPlugin.cxx
  qSlicerSubjectHierarchyDefaultPlugin.h
  qSlicerSubjectHierarchyCloneNodePlugin.cxx
  qSlicerSubjectHierarchyCloneNodePlugin.h
  qSlicerSubjectHierarchyChartsPlugin.cxx
  qSlicerSubjectHierarchyChartsPlugin.h
  qSlicerSubjectHierarchyParseLocalDataPlugin.cxx
  qSlicerSubjectHierarchyParseLocalDataPlugin.h
  qSlicerSubjectHierarchyRegisterPlugin.cxx
  qSlicerSubjectHierarchyRegisterPlugin.h
  qSlicerSubjectHierarchyFolderPlugin.cxx
  qSlicerSubjectHierarchyFolderPlugin.h
  qSlicerSubjectHierarchyOpacityPlugin.cxx
  qSlicerSubjectHierarchyOpacityPlugin.h
  qSlicerSubjectHierarchyVisibilityPlugin.cxx
  qSlicerSubjectHierarchyVisibilityPlugin.h
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND ${KIT}_SRCS
    qSlicerSubjectHierarchyScriptedPlugin.cxx
    qSlicerSubjectHierarchyScriptedPlugin.h
    )
endif()

set(${KIT}_MOC_SRCS
  qSlicerSubjectHierarchyPluginLogic.h
  qSlicerSubjectHierarchyAbstractPlugin.h
  qSlicerSubjectHierarchyPluginHandler.h
  qSlicerSubjectHierarchyDefaultPlugin.h
  qSlicerSubjectHierarchyCloneNodePlugin.h
  qSlicerSubjectHierarchyChartsPlugin.h
  qSlicerSubjectHierarchyParseLocalDataPlugin.h
  qSlicerSubjectHierarchyRegisterPlugin.h
  qSlicerSubjectHierarchyFolderPlugin.h
  qSlicerSubjectHierarchyOpacityPlugin.h
  qSlicerSubjectHierarchyVisibilityPlugin.h
  qMRMLSubjectHierarchyTreeView.h
  qMRMLSubjectHierarchyComboBox.h
  qMRMLSubjectHierarchyModel.h
  qMRMLSortFilterSubjectHierarchyProxyModel.h
  )
if(Slicer_USE_PYTHONQT)
  list(APPEND ${KIT}_MOC_SRCS
    qSlicerSubjectHierarchyScriptedPlugin.h
    )
endif()

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

#-----------------------------------------------------------------------------
set_source_files_properties( qSlicerSubjectHierarchyAbstractPlugin.h WRAP_EXCLUDE )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicerTerminologiesModuleWidgets
  MRMLCore
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND ${KIT}_TARGET_LIBRARIES
    MRMLCLI
  )
endif()

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
if(Slicer_USE_PYTHONQT)
  add_subdirectory(Python)
endif()

#-----------------------------------------------------------------------------
if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()
