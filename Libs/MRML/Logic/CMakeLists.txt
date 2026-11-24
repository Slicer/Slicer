# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKCommon
)
include(${ITK_USE_FILE})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
slicer_generate_module_wrapper(
  HEADER_NAME "vtkMRMLLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

set(classes
  vtkMRMLAbstractLogic
  vtkMRMLApplicationLogic
  vtkMRMLColorLogic
  vtkMRMLDisplayableHierarchyLogic
  vtkMRMLRemoteIOLogic
  vtkMRMLLayoutLogic
  vtkMRMLSliceLayerLogic
  vtkMRMLSliceLogic
  vtkMRMLSliceLinkLogic
  vtkMRMLViewLogic
  vtkMRMLViewLinkLogic
  vtkMRMLModuleLogic
  vtkMRMLFileIOHandler
  vtkMRMLFileIOManager
  vtkMRMLFileReader
  vtkMRMLFileWriter
  vtkMRMLIOOptionsDescription
  vtkMRMLIOProperties
  vtkMRMLOperationProgress
  vtkMRMLNodeWriter
  vtkMRMLSceneBundleReader
  # slicer's vtk extensions (filters)
  vtkImageLabelOutline
  vtkImageNeighborhoodFilter
)

vtk_module_add_module(Slicer::MRMLLogic
  EXPORT_MACRO_PREFIX VTK_MRML_LOGIC
  CLASSES ${classes}
  HEADERS "${wrapper_header_file}"
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::MRMLLogic PUBLIC ${ITK_LIBRARIES})

vtk_module_set_property(Slicer::MRMLLogic PROPERTY FOLDER VALUE MRMLLogic)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
