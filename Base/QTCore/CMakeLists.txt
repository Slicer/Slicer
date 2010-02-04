PROJECT(qSlicerBaseQTCore)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTCORE_EXPORT")

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
  qSlicerCoreApplication.cxx
  qSlicerCoreApplication.h
  qSlicerCoreCommandOptions.cxx
  qSlicerCoreCommandOptions.h
  qSlicerCoreIOManager.cxx
  qSlicerCoreIOManager.h
  qSlicerLoadableModuleFactory.cxx
  qSlicerLoadableModuleFactory.h
  qSlicerModuleManager.cxx
  qSlicerModuleManager.h
  qSlicerModuleFactoryManager.cxx
  qSlicerModuleFactoryManager.h
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerCoreApplication.h
  qSlicerModuleManager.h
  )

# UI files
#SET(slicer_qtbase_UI_SRCS)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  ModuleDescriptionParser
  MRMLCLI
  qSlicerBaseQTBase
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerBaseQTCore.qrc
)

Slicer3_build_slicer_qtbase_library(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${slicer_qtbase_export_directive}
  INCLUDE_DIRECTORIES ${slicer_qtbase_include_directories}
  SRCS ${slicer_qtbase_SRCS}
  MOC_SRCS ${slicer_qtbase_MOC_SRCS}
  #UI_SRCS ${slicer_qtbase_UI_SRCS}
  TARGET_LIBRARIES ${slicer_qtbase_target_libraries}
  RESOURCES ${qt_module_resources}
  )

# Sub directories
#ADD_SUBDIRECTORY(Plugins)
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
