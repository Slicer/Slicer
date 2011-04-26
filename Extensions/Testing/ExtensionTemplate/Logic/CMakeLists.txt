PROJECT(vtkSlicer${EXTENSION_NAME}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_SLICER_EXTENSIONTEMPLATE_MODULE_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerExtensionTemplateLogic.cxx
  vtkSlicerExtensionTemplateLogic.h
  )

# Additional Target libraries
SET(module_logic_target_libraries
  ${CTK_LIBRARIES}
  ${ITK_LIBRARIES}
  ${QT_LIBRARIES}
  ${VTK_LIBRARIES}
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  DISABLE_WRAP_PYTHON
  )
