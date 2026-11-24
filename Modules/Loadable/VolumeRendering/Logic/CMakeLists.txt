set(classes
  vtkSlicerShaderPropertyReader
  vtkSlicerVolumeRenderingLogic
  vtkSlicerVolumeRenderingReader
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerVolumeRenderingModuleLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::VolumeRenderingModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_VOLUMERENDERING_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_set_property(Slicer::VolumeRenderingModuleLogic PROPERTY FOLDER VALUE Module-VolumeRendering)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/presets.xml
  ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/VolumeRendering/presets.xml
  COPYONLY)

install(
  FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/VolumeRendering/presets.xml
  DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/VolumeRendering COMPONENT Runtime)
