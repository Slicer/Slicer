# Module name (usually identique to the directory name)
SET(qt_module_name "Models")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_MODELS_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
# Is used by the Plugins and Testing directories
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

# Source files
SET(qt_module_SRCS
  qSlicerModelsIO.cxx
  qSlicerModelsIO.h
  qSlicerModelsModule.cxx
  qSlicerModelsModule.h
  qSlicerModelsModuleWidget.cxx
  qSlicerModelsModuleWidget.h
  Widgets/qMRMLModelDisplayNodeWidget.cxx
  Widgets/qMRMLModelDisplayNodeWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerModelsIO.h
  qSlicerModelsModule.h
  qSlicerModelsModuleWidget.h
  Widgets/qMRMLModelDisplayNodeWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerModelsModule.ui
  Resources/UI/qMRMLModelDisplayNodeWidget.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerModelsModule.qrc
)

Slicer_build_qtmodule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

ADD_SUBDIRECTORY(Widgets/Plugins)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Widgets/Testing)
  #ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
