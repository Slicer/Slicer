PROJECT(SlicerBaseQTGUI)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTGUI_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${SlicerBaseQTBase_SOURCE_DIR}
  ${SlicerBaseQTBase_BINARY_DIR}
  ${SlicerBaseQTCLI_SOURCE_DIR}
  ${SlicerBaseQTCLI_BINARY_DIR}
  ${SlicerBaseQTCoreModules_SOURCE_DIR}
  ${SlicerBaseQTCoreModules_BINARY_DIR}
  )
  
# Source files
SET(slicer_qtbase_SRCS
  qSlicerAbstractModulePanel.cxx
  qSlicerAbstractModulePanel.h
  qSlicerApplication.cxx
  qSlicerApplication.h
  qSlicerModuleFactory.cxx
  qSlicerModuleFactory.h
  qSlicerModuleManager.cxx
  qSlicerModuleManager.h
  qSlicerModulePanel.cxx
  qSlicerModulePanel.h
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerAbstractModulePanel.h
  qSlicerApplication.h
  qSlicerModuleManager.h
  qSlicerModulePanel.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  SlicerBaseQTCLI
  SlicerBaseQTCoreModules
  SlicerBaseQTBase
  )

Slicer3_build_slicer_qtbase_library( 
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${slicer_qtbase_export_directive}
  INCLUDE_DIRECTORIES ${slicer_qtbase_include_directories}
  SRCS ${slicer_qtbase_SRCS}
  MOC_SRCS ${slicer_qtbase_MOC_SRCS}
  UI_SRCS ${slicer_qtbase_UI_SRCS}
  TARGET_LIBRARIES ${slicer_qtbase_target_libraries}
  )

# Sub directories
ADD_SUBDIRECTORY(Plugins)
