PROJECT(SlicerBaseQTCLI)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTCLI_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${SlicerBaseQTBase_SOURCE_DIR}
  ${SlicerBaseQTBase_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  )

# Source files
SET(slicer_qtbase_SRCS
  qSlicerCLIModule.h
  qSlicerCLIModule.cxx
  qSlicerCLIModuleLogic.h
  qSlicerCLIModuleLogic.cxx
  #ui_qSlicerCLIModule.h
  #ui_qSlicerCLIModule.cxx
  #qSlicerCLIModuleUILoader.h
  #qSlicerCLIModuleUILoader.cxx
  #qSlicerExecutableCLIModuleLogic.h
  #qSlicerExecutableCLIModuleLogic.cxx
  #qSlicerPythonCLIModuleLogic.h
  #qSlicerPythonCLIModuleLogic.cxx
  vtkMRMLCommandLineModuleNode.h
  vtkMRMLCommandLineModuleNode.cxx
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerCLIModule.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  Resources/UI/qSlicerCLIModule.ui
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  qCTKWidgets
  SlicerBaseQTBase
  SlicerBaseLogic
  ModuleDescriptionParser
  )

Slicer3_build_slicer_qtbase_library(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${slicer_qtbase_export_directive}
  INCLUDE_DIRECTORIES ${slicer_qtbase_include_directories}
  SRCS ${slicer_qtbase_SRCS}
  MOC_SRCS ${slicer_qtbase_MOC_SRCS}
  UI_SRCS ${slicer_qtbase_UI_SRCS}
  TARGET_LIBRARIES ${slicer_qtbase_target_libraries}
  NO_RESOURCES
  )

# --------------------------------------------------------------------------
# Subdirectories
#
#ADD_SUBDIRECTORY(Plugins)
