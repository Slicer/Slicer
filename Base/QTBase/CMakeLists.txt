PROJECT(SlicerBaseQTBase)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTBASE_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  )
  
# Source files
SET(slicer_qtbase_SRCS
  qSlicerWidget.h
  qSlicerWidget.cxx
  qSlicerUtils.h
  qSlicerUtils.cxx
  qSlicerAbstractModule.h
  qSlicerAbstractModule.cxx
  qSlicerAbstractCoreModule.h
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractLoadableModule.h
  qSlicerAbstractLoadableModule.cxx
  qSlicerModuleLogic.h
  qSlicerModuleLogic.cxx
  qSlicerIOManager.h
  qSlicerIOManager.cxx
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerWidget.h
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.h
  qSlicerAbstractLoadableModule.h
  qSlicerModuleLogic.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  #Resources/UI/xxx.ui
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
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
