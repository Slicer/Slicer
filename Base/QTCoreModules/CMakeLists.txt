PROJECT(qSlicerBaseQTCoreModules)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTCOREMODULES_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${qSlicerBaseQTBase_SOURCE_DIR}
  ${qSlicerBaseQTBase_BINARY_DIR}
  )
  
# Source files
SET(slicer_qtbase_SRCS
  qSlicerCoreModuleFactory.cxx
  qSlicerCoreModuleFactory.h
  
  qSlicerCamerasModule.cxx
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.cxx
  qSlicerCamerasModuleWidget.h
  
  qSlicerTransformsModule.cxx
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.cxx
  qSlicerTransformsModuleWidget.h
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerCamerasModule.h
  qSlicerCamerasModuleWidget.h
  
  qSlicerTransformsModule.h
  qSlicerTransformsModuleWidget.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  Resources/UI/qSlicerTransformsModule.ui
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
  qSlicerBaseQTBase
  qSlicerBaseQTCore
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerBaseQTCoreModules.qrc
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

# Sub directories
#ADD_SUBDIRECTORY(Plugins)
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
