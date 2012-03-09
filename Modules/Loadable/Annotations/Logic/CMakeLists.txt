project(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_ANNOTATIONS_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/../GUI
  ${CMAKE_CURRENT_BINARY_DIR}/../GUI
  )

# Source files
set(module_logic_SRCS
  vtkSlicerAnnotationModuleLogic.cxx
  vtkSlicerAnnotationModuleLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  vtkSlicerAnnotationsModuleMRML
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
