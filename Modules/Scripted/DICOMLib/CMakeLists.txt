#-----------------------------------------------------------------------------
set(MODULE_NAME DICOMLib)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Logic)
add_subdirectory(Widgets)
add_subdirectory(SubjectHierarchyPlugins)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  __init__
  DICOMBrowser
  DICOMExportScalarVolume
  DICOMExportScene
  DICOMPlugin
  DICOMPluginSelector
  DICOMProcesses
  DICOMRecentActivityWidget
  DICOMSendDialog
  DICOMUtils
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_SUBDIR
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()
