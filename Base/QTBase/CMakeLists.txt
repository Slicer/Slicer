PROJECT(qSlicerBaseQTBase)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTBASE_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  )
  
# Source files
SET(KIT_SRCS
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
SET(KIT_MOC_SRCS
  qSlicerWidget.h
  qSlicerAbstractModule.h
  qSlicerAbstractCoreModule.h
  qSlicerAbstractLoadableModule.h
  qSlicerAbstractModuleWidget.h
  )

# UI files
SET(KIT_UI_SRCS
  #Resources/UI/xxx.ui
)

# Resources
SET(KIT_resources
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  SlicerBaseLogic
  qMRMLWidgets
  )

Slicer3_build_slicer_qtbase_library( 
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  )

# Plugins
#ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
