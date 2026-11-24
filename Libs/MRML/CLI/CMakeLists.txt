vtk_module_find_package(
  PACKAGE SlicerExecutionModel
  COMPONENTS ModuleDescriptionParser
  PRIVATE_IF_SHARED
)

# ITK - Import ITK targets required by ModuleDescriptionParser
vtk_module_find_package(
  PACKAGE ITK
  COMPONENTS ${ModuleDescriptionParser_ITK_COMPONENTS}
)

slicer_generate_module_wrapper(
  HEADER_NAME "vtkMRMLCLIExport.h"
  OUTPUT_VAR wrapper_header_file
)

set(classes vtkMRMLCommandLineModuleNode)

vtk_module_add_module(Slicer::MRMLCLI
  EXPORT_MACRO_PREFIX VTK_MRML_CLI
  CLASSES ${classes}
  HEADERS "${wrapper_header_file}"
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::MRMLCLI PUBLIC ${ITK_LIBRARIES} PRIVATE ModuleDescriptionParser)
vtk_module_include(Slicer::MRMLCLI PRIVATE ${ModuleDescriptionParser_INCLUDE_DIRS})
vtk_module_set_property(Slicer::MRMLCLI PROPERTY FOLDER VALUE MRMLCLI)
