set(classes
  vtkMRMLLayerDMCameraSynchronizer
  vtkMRMLLayerDMInteractionLogic
  vtkMRMLLayerDMLayerManager
  vtkMRMLLayerDMPipelineCallbackCreator
  vtkMRMLLayerDMPipelineCreatorI
  vtkMRMLLayerDMPipelineFactory
  vtkMRMLLayerDMPipelineI
  vtkMRMLLayerDMPipelineManager
  vtkMRMLLayerDisplayableManager
  vtkMRMLLayerDMPipelineScriptedCreator
  vtkMRMLLayerDMScriptedPipelineBridge
)

set(headers
  vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h
)

# Handle factory registration
set(factory_classes
  vtkMRMLLayerDisplayableManager
)

foreach(class IN LISTS factory_classes)
  vtk_object_factory_declare(BASE ${class} OVERRIDE ${class})
endforeach()

set_property(GLOBAL PROPERTY "_vtk_module_autoinit_include" "\"vtkAutoInit.h\"")
vtk_object_factory_configure(
  SOURCE_FILE vtk_object_factory_source
  HEADER_FILE vtk_object_factory_header
  EXPORT_MACRO "")

# Module configuration
vtk_module_add_module(SlicerLayerDM::MRMLDisplayableManager
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER
  CLASSES ${classes}
  SOURCES ${vtk_object_factory_source}
  HEADERS ${headers}
  PRIVATE_HEADERS ${vtk_object_factory_header}
)

vtk_module_definitions(SlicerLayerDM::MRMLDisplayableManager PRIVATE
  "vtkSlicerLayerDMModuleMRMLDisplayableManager_AUTOINIT=1(vtkSlicerLayerDMModuleMRMLDisplayableManager)")

add_subdirectory(Python)
