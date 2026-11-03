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
set(configure_header_file vtkMRMLDisplayableManagerConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkMRMLDisplayableManagerExport.h"
  OUTPUT_VAR wrapper_header_file
  ADDITIONAL_INCLUDES ${configure_header_file}
)

set(classes
  # DisplayableManager associated with ThreeDView
  vtkMRMLCameraDisplayableManager
  vtkMRMLModelDisplayableManager
  vtkMRMLViewDisplayableManager
  vtkMRMLThreeDReformatDisplayableManager
  vtkMRMLThreeDSliceEdgeDisplayableManager

  # DisplayableManager associated with SliceView
  vtkMRMLCrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager3D
  vtkMRMLModelSliceDisplayableManager
  vtkMRMLVolumeGlyphSliceDisplayableManager

  # DisplayableManager common between ThreeDView and SliceView
  vtkMRMLAccuratePicker
  vtkMRMLOrientationMarkerDisplayableManager
  vtkMRMLRulerDisplayableManager
  vtkMRMLScalarBarDisplayableManager
  vtkMRMLAbstractDisplayableManager
  vtkMRMLDisplayableManagerGroup
  vtkMRMLDisplayableManagerFactory

  # ThreeDView factory and DisplayableManager
  vtkMRMLAbstractThreeDViewDisplayableManager
  vtkMRMLThreeDViewDisplayableManagerFactory

  # SliceView factory and DisplayableManager
  vtkMRMLAbstractSliceViewDisplayableManager
  vtkMRMLSliceViewDisplayableManagerFactory
  vtkMRMLViewInteractorStyle
  vtkMRMLSliceViewInteractorStyle
  vtkMRMLThreeDViewInteractorStyle
  vtkMRMLAccuratePicker
  vtkMRMLRayCastMapper

  # Widgets
  vtkMRMLAbstractWidget
  vtkMRMLAbstractWidgetRepresentation
  vtkMRMLCameraWidget
  vtkMRMLInteractionWidget
  vtkMRMLInteractionWidgetRepresentation
  vtkMRMLSliceEdgeWidgetRepresentation
  vtkMRMLSliceIntersectionWidget
  vtkMRMLSliceIntersectionRepresentation2D
  vtkMRMLSliceIntersectionInteractionRepresentation
  vtkMRMLSliceIntersectionInteractionRepresentationHelper
  vtkMRMLRubberBandWidgetRepresentation
  vtkMRMLWindowLevelWidget
)

set_source_files_properties(
  vtkMRMLDisplayableManagerGroup.cxx
  vtkMRMLScriptedDisplayableManager.cxx
  WRAP_EXCLUDE
)

if(VTK_WRAP_PYTHON)
  list(APPEND classes vtkMRMLScriptedDisplayableManager)
  set_source_files_properties(
    vtkMRMLScriptedDisplayableManager.cxx
    WRAP_EXCLUDE
  )
endif()

set(headers
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  ${wrapper_header_file}
)

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------

set(factory_classes
  vtkMRMLCameraDisplayableManager
  vtkMRMLModelDisplayableManager
  vtkMRMLViewDisplayableManager
  vtkMRMLThreeDReformatDisplayableManager
  vtkMRMLThreeDSliceEdgeDisplayableManager
  # DisplayableManager associated with SliceView
  vtkMRMLCrosshairDisplayableManager
  vtkMRMLCrosshairDisplayableManager3D
  vtkMRMLModelSliceDisplayableManager
  vtkMRMLVolumeGlyphSliceDisplayableManager
  # DisplayableManager common between ThreeDView and SliceView
  vtkMRMLOrientationMarkerDisplayableManager
  vtkMRMLRulerDisplayableManager
  vtkMRMLScalarBarDisplayableManager
  # ThreeDView DisplayableManager
  vtkMRMLAbstractThreeDViewDisplayableManager
  # SliceView DisplayableManager
  vtkMRMLAbstractSliceViewDisplayableManager
)
foreach(class IN LISTS factory_classes)
  vtk_object_factory_declare(BASE ${class} OVERRIDE ${class})
endforeach()

vtk_object_factory_configure(
  SOURCE_FILE vtk_object_factory_source
  HEADER_FILE vtk_object_factory_header
  EXPORT_MACRO "VTK_MRML_DISPLAYABLEMANAGER_EXPORT"
)

set_property(GLOBAL PROPERTY "_vtk_module_autoinit_include" "\"vtkAutoInit.h\"")
vtk_module_add_module(Slicer::MRMLDisplayableManager
  EXPORT_MACRO_PREFIX VTK_MRML_DISPLAYABLEMANAGER
  CLASSES ${classes}
  SOURCES ${vtk_object_factory_source}
  HEADERS ${headers}
  PRIVATE_HEADERS ${vtk_object_factory_header}
  EXCLUDE_HEADER_TEST
)

vtk_module_definitions(Slicer::MRMLDisplayableManager PRIVATE MRMLDisplayableManager_USE_PYTHON)
vtk_module_link(Slicer::MRMLDisplayableManager
  PUBLIC ${ITK_LIBRARIES}
  PRIVATE VTK::WrappingPythonCore Python3::Module
)

vtk_module_set_property(Slicer::MRMLDisplayableManager PROPERTY FOLDER VALUE MRMLDisplayableManager)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
