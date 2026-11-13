#-----------------------------------------------------------------------------
set(MODULE_NAME "TemplateKey")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(Widgets)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${qSlicerMarkupsModuleWidgets_INCLUDE_DIRS}
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  )

set(MODULE_TARGET_LIBRARIES
  Slicer::TemplateKeyModuleMRML
  Slicer::TemplateKeyModuleLogic
  qSlicer${MODULE_NAME}ModuleWidgets
  Slicer::MarkupsModuleLogic
  qSlicerMarkupsModuleWidgets
  )

#-----------------------------------------------------------------------------
slicerMacroBuildLoadableModule(
  NAME ${MODULE_NAME}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  # Important: remove NO_INSTALL option below when this template is used for implementing a new module.
  # Installation is disabled here because this template is used for testing custom markups in Slicer core
  # and we don't want this testing module to appear in the Slicer installation package.
  NO_INSTALL
  )
