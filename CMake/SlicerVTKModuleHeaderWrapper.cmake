#[==[.rst:
.. cmake:command:: slicer_generate_module_wrapper

  Generate a compatibility header that enables Slicer code to still be compiled as VTK modules.

  - `HEADER_NAME`: Name of generated header, should be the name of the export header used in Slicer codebase.
  - `OUTPUT_VAR`: Optional output variable name, if specified it will be set to the path of header file.
  - `ADDITIONAL_INCLUDES`: Optional list of header names to include. They are wrapped with `<` and `>`.

  For example, in Slicer::MRMLCore module, old export header was named "vtkMRMLCoreExport.h",
  but with given module library name, the generated export header is "MRMLCoreModule.h", so the command would be:
  `slicer_generate_module_wrapper(Slicer::MRMLCore "vtkMRMLCoreExport.h")`
  "vtkMRMLCoreExport.h" will include "MRMLCoreModule.h".

  Note that this command must only be called in the CMakeLists.txt of a VTK module!

  .. code-block:: cmake

  slicer_generate_module_wrapper(
    HEADER_NAME <name>
    [OUTPUT_VAR <name>]
    [ADDITIONAL_INCLUDES <names...>]
    )
#]==]
function(slicer_generate_module_wrapper)
  cmake_parse_arguments(PARSE_ARGV 0 arg
    ""
    "HEADER_NAME;OUTPUT_VAR"
    "ADDITIONAL_INCLUDES"
  )

  if(NOT arg_HEADER_NAME)
    message(FATAL_ERROR "HEADER_NAME must be specified and non empty!")
  endif()

  set(module_file "${CMAKE_CURRENT_SOURCE_DIR}/vtk.module")
  if(NOT EXISTS ${module_file})
    message(FATAL_ERROR "Could not find a vtk.module in current source directory."
      "Please ensure that this function is only called on a VTK module CMakeLists.txt!")
  endif()

  # At the time this function is called, we don't have the real target defined,
  # so lets grab the info from the actual source
  file(READ "${module_file}" module_descr)
  string(REGEX REPLACE "#[^\n]*\n" "\n" module_descr "${module_descr}") # comments
  string(REGEX REPLACE "( |\n)+" ";" module_descr "${module_descr}") # make it a cmake args list
  _vtk_module_parse_module_args(module_name ${module_descr})
  set(lib_name "${${module_name}_LIBRARY_NAME}")

  # ${HEADER_NAME} -> ${lib_name}Module.h
  set(content)
  string(APPEND content "#ifndef ${lib_name}Module_WRAPPER_H_INCLUDE\n")
  string(APPEND content "#define ${lib_name}Module_WRAPPER_H_INCLUDE\n")
  string(APPEND content "#include <${lib_name}Module.h>\n")
  foreach(file IN LISTS arg_ADDITIONAL_INCLUDES)
    string(APPEND content "#include <${file}>\n")
  endforeach()
  string(APPEND content "#endif\n")

  set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${arg_HEADER_NAME}")
  file(WRITE "${output_file}" "${content}")

  if(arg_OUTPUT_VAR)
    set("${arg_OUTPUT_VAR}" "${output_file}" PARENT_SCOPE)
  endif()
endfunction()
