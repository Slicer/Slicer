PROJECT(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_SLICER_CROPVOLUME_MODULE_LOGIC_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  ${qt_module_include_directories}
  )

# Source files
SET(module_logic_SRCS
  vtkSlicerCropVolumeLogic.cxx ${CMAKE_CURRENT_SOURCE_DIR}/../MRML/vtkMRMLCropVolumeParametersNode.cxx
  )

# Additional Target libraries
SET(module_logic_target_libraries
  ${ITK_LIBRARIES}
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
