set(KIT ${PROJECT_NAME}StylePlugins)
project(${KIT})

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}Export.h' will be automatically configured.
set(${KIT}_EXPORT "Q_SLICER_STYLES_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qSlicerStylePlugin.cxx
  qSlicerStylePlugin.h
  )

# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qSlicerStylePlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicerBaseQTGUI
  )

ctkMacroBuildQtStylesPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Core-Base"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
