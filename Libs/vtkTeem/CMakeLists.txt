# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS
    ITKCommon
    ITKVNL
)
include(${ITK_USE_FILE})

vtk_module_find_package(PACKAGE vtkAddon)

vtk_module_find_package(PACKAGE Teem PRIVATE_IF_SHARED)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
slicer_generate_module_wrapper(
  HEADER_NAME "vtkTeemExport.h"
  OUTPUT_VAR wrapper_header_file
)

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(classes
  vtkDiffusionTensorMathematics
  vtkDiffusionTensorGlyph
  vtkTeemNRRDReader
  vtkTeemNRRDWriter
  vtkImageLabelCombine
  )

vtk_module_add_module(Slicer::Teem
  EXPORT_MACRO_PREFIX VTK_Teem
  CLASSES ${classes}
  HEADERS "${wrapper_header_file}"
  EXCLUDE_HEADER_TEST
)

vtk_module_include(Slicer::Teem PRIVATE ${Teem_INCLUDE_DIRS})

vtk_module_link(Slicer::Teem
  PUBLIC
    ${ITK_LIBRARIES}
    vtkAddon
  PRIVATE
    ${Teem_LIBRARIES}
)

vtk_module_set_property(Slicer::Teem PROPERTY FOLDER VALUE vtkTeem)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
