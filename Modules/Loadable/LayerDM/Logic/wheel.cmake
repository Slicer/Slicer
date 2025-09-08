set(LayerDM_VERSION "${SKBUILD_PROJECT_VERSION}")
string(REPLACE "." ";" version_components "${LayerDM_VERSION}") # this creates a list
list(GET version_components 0 LayerDM_VERSION_MAJOR)
list(GET version_components 1 LayerDM_VERSION_MINOR)
list(GET version_components 2 LayerDM_VERSION_PATCH)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerLayerDMVersion.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerLayerDMVersion.h"
  @ONLY
)

set(classes vtkSlicerLayerDMLogic)

set(headers
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerLayerDMVersion.h"
)

vtk_module_add_module(SlicerLayerDM::Logic
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${headers}
)
