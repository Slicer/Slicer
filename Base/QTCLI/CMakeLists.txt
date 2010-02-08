PROJECT(qSlicerBaseQTCLI)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTCLI_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${qSlicerBaseQTBase_SOURCE_DIR}
  ${qSlicerBaseQTBase_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  )

# Source files
SET(slicer_qtbase_SRCS
  qSlicerCLIExecutableModuleFactory.cxx
  qSlicerCLIExecutableModuleFactory.h
  qSlicerCLILoadableModule.cxx
  qSlicerCLILoadableModule.h
  qSlicerCLILoadableModuleFactory.cxx
  qSlicerCLILoadableModuleFactory.h
  qSlicerCLIModule.cxx
  qSlicerCLIModule.h
  qSlicerCLIModuleFactoryHelper.cxx
  qSlicerCLIModuleFactoryHelper.h
  qSlicerCLIModuleWidget.cxx
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  vtkSlicerCLIModuleLogic.cxx
  vtkSlicerCLIModuleLogic.h
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  Resources/UI/qSlicerCLIModule.ui
)

# Resources
SET(qt_module_resources
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  qCTKWidgets
  qSlicerBaseQTBase
  qSlicerBaseQTCore
  SlicerBaseLogic
  ModuleDescriptionParser
  MRMLCLI
  )

Slicer3_build_slicer_qtbase_library(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${slicer_qtbase_export_directive}
  INCLUDE_DIRECTORIES ${slicer_qtbase_include_directories}
  SRCS ${slicer_qtbase_SRCS}
  MOC_SRCS ${slicer_qtbase_MOC_SRCS}
  UI_SRCS ${slicer_qtbase_UI_SRCS}
  TARGET_LIBRARIES ${slicer_qtbase_target_libraries}
  RESOURCES ${qt_module_resources}
  )

# --------------------------------------------------------------------------
# Subdirectories
#
#ADD_SUBDIRECTORY(Plugins)
