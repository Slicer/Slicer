# Module name (usually identique to the directory name)
SET(qt_module_name "Welcome")

SET(qt_module_title "Welcome to Slicer")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_WELCOME_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  )

# Source files
SET(qt_module_SRCS
  qSlicerWelcomeModule.cxx
  qSlicerWelcomeModule.h
  qSlicerWelcomeModuleWidget.cxx
  qSlicerWelcomeModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerWelcomeModule.h
  qSlicerWelcomeModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerWelcomeModule.ui
)

# Resources
SET(qt_module_resources
  Resources/qSlicerWelcomeModule.qrc
)

# Additional Target libraries
SET(qt_module_target_libraries
  )


slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  TITLE ${qt_module_title}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

IF(BUILD_TESTING)
  add_subdirectory(Testing)
ENDIF()
