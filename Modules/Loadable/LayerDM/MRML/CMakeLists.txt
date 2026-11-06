set(classes
  vtkMRMLLayerDMNodeReferenceObserver
  vtkMRMLLayerDMObjectEventObserver
  vtkMRMLLayerDMWidgetEventTranslationNode
  vtkMRMLLayerDMSelectionObserver
  vtkMRMLLayerDMObjectEventObserverScripted
  vtkMRMLLayerDMPythonUtil
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerLayerDMModuleMRMLExport.h"
  OUTPUT_VAR wrapper_header_file
)

vtk_module_add_module(Slicer::LayerDMModuleMRML
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)
