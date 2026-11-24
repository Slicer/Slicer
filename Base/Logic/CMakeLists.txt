# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE SlicerExecutionModel
  COMPONENTS ModuleDescriptionParser
  PRIVATE_IF_SHARED
)
vtk_module_find_package(
  PACKAGE LibArchive
  PRIVATE_IF_SHARED
)

vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKCommon
    ITKIOTransformBase
    ITKTransform
    ${ModuleDescriptionParser_ITK_COMPONENTS}
)
include(${ITK_USE_FILE})

# Register the default ITK factories (see Libs/ITKFactoryRegistration).
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)

vtk_module_find_package(PACKAGE vtkAddon)

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
add_subdirectory(Resources)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkSlicerBaseLogicExport.h"
  OUTPUT_VAR wrapper_header_file
)

set(classes
  # Logic classes (data management and calculation)
  vtkSlicerApplicationLogic
  vtkSlicerModuleLogic
  vtkSlicerTask
  vtkDataIOManagerLogic
  # Slicer's VTK extensions (filters)
  vtkTransformVisualizerGlyph3D
  vtkImageRectangularSource
  vtkSystemInformation
  vtkImageFillROI
  vtkSlicerCLIModuleLogic
)

if(Slicer_USE_PYTHONQT)
  list(APPEND classes
    vtkSlicerScriptedFileReaderBridge
    vtkSlicerScriptedFileWriterBridge
    vtkSlicerScriptedLoadableModuleLogic
  )
endif()

set(headers
  vtkSlicerBaseLogic.h
  "${wrapper_header_file}"
)

vtk_module_add_module(Slicer::BaseLogic
  EXPORT_MACRO_PREFIX VTK_SLICER_BASE_LOGIC
  CLASSES ${classes}
  HEADERS ${headers}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::BaseLogic
  PUBLIC
    ${ITK_LIBRARIES} vtkAddon
  PRIVATE
    ModuleDescriptionParser
    ${LibArchive_LIBRARY}
)

vtk_module_include(Slicer::BaseLogic
  PRIVATE
    ${ModuleDescriptionParser_INCLUDE_DIRS}
    ${LibArchive_INCLUDE_DIRS}
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
