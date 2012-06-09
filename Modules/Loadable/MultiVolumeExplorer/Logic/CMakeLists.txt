project(vtkSlicer${EXTENSION_NAME}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/../MRML
  ${CMAKE_CURRENT_BINARY_DIR}/../MRML
  )

# Source files
set(module_logic_SRCS
  vtkSlicerMultiVolumeExplorerLogic.cxx
  vtkSlicerMultiVolumeExplorerLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  # ${ITK_LIBRARIES}
  vtkSlicerMultiVolumeExplorerModuleMRML
  vtkSlicerVolumesModuleLogic
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
