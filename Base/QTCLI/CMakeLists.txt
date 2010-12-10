PROJECT(qSlicerBaseQTCLI)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTCLI_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  )

# Source files
SET(KIT_SRCS
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
  qSlicerCLIModuleUIHelper.cxx
  qSlicerCLIModuleUIHelper.h
  qSlicerCLIModuleWidget.cxx
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  vtkSlicerCLIModuleLogic.cxx
  vtkSlicerCLIModuleLogic.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  qSlicerCLIModuleUIHelper.h
  )

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerCLIModuleWidget.ui
)

# Resources
SET(KIT_resources
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  qSlicerBaseQTCore
  qSlicerBaseQTGUI
  qMRMLWidgets # HACK: Added ONLY for qCTKFlowLayout
  #SlicerBaseLogic
  ModuleDescriptionParser
  MRMLCLI
  )

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  )

# Plugin
#ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
