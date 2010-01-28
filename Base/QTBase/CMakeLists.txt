PROJECT(qSlicerBaseQTBase)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTBASE_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  )
  
# Source files
SET(slicer_qtbase_SRCS
  qSlicerWidget.cxx
  qSlicerWidget.h
  qSlicerUtils.cxx
  qSlicerUtils.h
  qSlicerAbstractModule.cxx
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractCoreModule.h
  qSlicerAbstractLoadableModule.cxx
  qSlicerAbstractLoadableModule.h
  qSlicerAbstractModuleWidget.cxx
  qSlicerAbstractModuleWidget.h
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerWidget.h
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.h
  qSlicerAbstractLoadableModule.h
  qSlicerAbstractModuleWidget.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  #Resources/UI/xxx.ui
)

# Resources
SET(qt_module_resources
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  SlicerBaseLogic
  qMRMLWidgets
  qVTKWidgets
  qCTKWidgets
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
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
