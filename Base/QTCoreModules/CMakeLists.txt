PROJECT(SlicerBaseQTCoreModules)

#
# See CMake/Slicer3QTBaseLibraryMacros.cmake for details
#

SET(slicer_qtbase_export_directive "Q_SLICER_BASE_QTCOREMODULES_EXPORT")

# Additional directories to include
SET(slicer_qtbase_include_directories
  ${SlicerBaseQTBase_SOURCE_DIR}
  ${SlicerBaseQTBase_BINARY_DIR}
  )
  
# Source files
SET(slicer_qtbase_SRCS
  qSlicerCamerasModule.h
  qSlicerCamerasModule.cxx
  qSlicerTransformsModule.h
  qSlicerTransformsModule.cxx
  )

# Headers that should run through moc
SET(slicer_qtbase_MOC_SRCS
  qSlicerCamerasModule.h
  qSlicerTransformsModule.h
  )

# UI files
SET(slicer_qtbase_UI_SRCS
  Resources/UI/qSlicerCamerasModule.ui
  Resources/UI/qSlicerTransformsModule.ui
)

# Additional Target libraries (QT libs already included)
SET(slicer_qtbase_target_libraries
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
#ADD_SUBDIRECTORY(Plugins)
