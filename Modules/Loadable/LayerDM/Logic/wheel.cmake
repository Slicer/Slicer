set(LayerDM_VERSION "${SKBUILD_PROJECT_VERSION}")
string(REPLACE "." ";" version_components "${LayerDM_VERSION}") # this creates a list
list(GET version_components 0 LayerDM_VERSION_MAJOR)
list(GET version_components 1 LayerDM_VERSION_MINOR)
list(GET version_components 2 LayerDM_VERSION_PATCH)

set(classes
  vtkSlicerLayerDMLogic
)

set(headers
  vtkSlicerLayerDMModuleLogicExport.h
)

vtk_module_add_module(Slicer::LayerDMModuleLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_LOGIC
  CLASSES ${classes}
  HEADERS ${headers}
)
