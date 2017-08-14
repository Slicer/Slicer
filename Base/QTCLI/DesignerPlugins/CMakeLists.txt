set(KIT ${PROJECT_NAME}Plugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_BASE_QTCLI_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qSlicerQTCLIAbstractPlugin.cxx
  qSlicerQTCLIAbstractPlugin.h
  qSlicerQTCLIPlugins.cxx
  qSlicerQTCLIPlugins.h

  qSlicerCLIProgressBarPlugin.cxx
  qSlicerCLIProgressBarPlugin.h
  )

# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerQTCLIPlugins.h
  qSlicerCLIProgressBarPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerBaseQTCLI
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Core-Base"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
