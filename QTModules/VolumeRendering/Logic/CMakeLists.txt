project(vtkSlicer${qt_module_name}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC_EXPORT")

# Additional directories to include
set(module_logic_include_directories
  ${vtkSlicerVolumeRenderingModuleMRML_SOURCE_DIR}
  ${vtkSlicerVolumeRenderingModuleMRML_BINARY_DIR}
  ${VolumeRenderingReplacements_SOURCE_DIR}
  ${VolumeRenderingReplacements_BINARY_DIR}
  )

# Source files
set(module_logic_SRCS
  vtkSlicerVolumeRenderingLogic.cxx
  vtkSlicerVolumeRenderingLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  vtkSlicerVolumeRenderingModuleMRML
  VolumeRenderingReplacements
  vtkSlicerAnnotationsModuleMRML
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/presets.xml
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/presets.xml
  COPYONLY)

install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/presets.xml
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR} COMPONENT Runtime)
