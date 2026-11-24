#-----------------------------------------------------------------------------
set(MODULE_NAME MultiVolumeExplorer)

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Py)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  )

set(MODULE_SRCS
  qSlicerMultiVolumeExplorerModule.cxx
  qSlicerMultiVolumeExplorerModule.h
  )

set(MODULE_MOC_SRCS
  qSlicerMultiVolumeExplorerModule.h
  )

set(MODULE_UI_SRCS
)

set(MODULE_TARGET_LIBRARIES
  Slicer::MultiVolumeExplorerModuleMRML
  Slicer::MultiVolumeExplorerModuleLogic
  )

set(MODULE_RESOURCES
  Resources/qSlicerMultiVolumeExplorerModule.qrc
)

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
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
  add_subdirectory(Testing)
endif()
