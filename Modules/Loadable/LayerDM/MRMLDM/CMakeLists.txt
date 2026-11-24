set(classes
  vtkMRMLLayerDMCameraSynchronizer
  vtkMRMLLayerDMInteractionLogic
  vtkMRMLLayerDMLayerManager
  vtkMRMLLayerDMPipeline
  vtkMRMLLayerDMPipelineCallbackCreator
  vtkMRMLLayerDMPipelineCreator
  vtkMRMLLayerDMPipelineFactory
  vtkMRMLLayerDMPipelineManager
  vtkMRMLLayerDisplayableManager
)

if(Slicer_USE_PYTHONQT)
  list(APPEND classes
    vtkMRMLLayerDMPipelineScriptedCreator
    vtkMRMLLayerDMScriptedPipelineBridge
  )
endif()

set(headers
  vtkMRMLLayerDMPipelineCreateHelper.h
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"
  OUTPUT_VAR wrapper_header_file
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
  EXPORT_MACRO "VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

# Module configuration
vtk_module_add_module(Slicer::LayerDMModuleMRMLDisplayableManager
  EXPORT_MACRO_PREFIX VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER
  CLASSES ${classes}
  SOURCES ${vtk_object_factory_source}
  HEADERS ${headers} ${wrapper_header_file}
  PRIVATE_HEADERS ${vtk_object_factory_header}
  EXCLUDE_HEADER_TEST
)

if(Slicer_USE_PYTHONQT)
  add_subdirectory(Python)
endif()
