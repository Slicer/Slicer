set(classes
  vtkMRMLLayerDMNodeReferenceObserver
  vtkMRMLLayerDMObjectEventObserver
  vtkMRMLLayerDMWidgetEventTranslationNode
  vtkMRMLLayerDMSelectionObserver
  vtkMRMLLayerDMObjectEventObserverScripted
  vtkMRMLLayerDMPythonUtil
)

set(headers
  vtkSlicerLayerDMModuleMRMLExport.h
)

vtk_module_add_module(SlicerLayerDM::MRML
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_MRML
  CLASSES ${classes}
  HEADERS ${headers}
)
