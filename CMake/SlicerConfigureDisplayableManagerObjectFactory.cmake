
#
# Configure Slicer displayable manager VTK object factory
#
# Given a list of displayable manager source files, this function will
# generate sources responsible for registering the classes with the
# vtkObjectFactory.
#
# This function set the variable "${TARGET_NAME}_AUTOINIT" to 1 in the parent
# scope. This tells SlicerMacroBuildModuleLogic to set Slicer_EXPORT_HEADER_CUSTOM_CONTENT
# and define the associated ${TARGET_NAME}_AUTOINIT compilation flag.
#
function(SlicerConfigureDisplayableManagerObjectFactory)
  set(options
    NO_EXPORT
    )
  set(oneValueArgs
    TARGET_NAME
    EXPORT_MACRO
    EXPORT_HEADER
    OUTPUT_SRCS_VAR
    )
  set(multiValueArgs
    SRCS
    )
  cmake_parse_arguments(DM_OBJECT_FACTORY
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # Sanity checks
  if(DM_OBJECT_FACTORY_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SlicerConfigureDisplayableManagerObjectFactory(): \"${DM_OBJECT_FACTORY_UNPARSED_ARGUMENTS}\"")
  endif()
  set(expected_nonempty_vars
    TARGET_NAME
    OUTPUT_SRCS_VAR
    SRCS
    )
  if(NOT DM_OBJECT_FACTORY_NO_EXPORT)
    list(APPEND expected_nonempty_vars
      EXPORT_MACRO
      EXPORT_HEADER
      )
  endif()
  foreach(var ${expected_nonempty_vars})
    if("${DM_OBJECT_FACTORY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  # Initialize local variables
  set(_vtk_override_includes)
  set(_vtk_override_creates )

  # For each class
  foreach(FILE ${DM_OBJECT_FACTORY_SRCS})

    # what is the filename without the extension
    get_filename_component(TMP_FILENAME ${FILE} NAME_WE)

    set(_class ${TMP_FILENAME})
    set(_override ${_class})

    set(_vtk_override_includes
      "${_vtk_override_includes}#include \"${_override}.h\"\n")

    set(_vtk_override_creates "${_vtk_override_creates}
      VTK_CREATE_CREATE_FUNCTION(${_override})")

    set(_vtk_override_do "${_vtk_override_do}
    this->RegisterOverride(\"${_class}\",
                           \"${_override}\",
                           \"Override for ${vtk-module} module\", 1,
                           vtkObjectFactoryCreate${_override});")

  endforeach()

  set(vtk-module ${DM_OBJECT_FACTORY_TARGET_NAME})

  if(DM_OBJECT_FACTORY_NO_EXPORT)
    string(TOUPPER ${DM_OBJECT_FACTORY_TARGET_NAME} VTK-MODULE)
    set(_export_header "#define ${VTK-MODULE}_EXPORT")
  else()
    string(REPLACE "_EXPORT" "" VTK-MODULE "${DM_OBJECT_FACTORY_EXPORT_MACRO}")
    set(_export_header "#include \"${DM_OBJECT_FACTORY_EXPORT_HEADER}\"")
  endif()
  set(CMAKE_CONFIGURABLE_FILE_CONTENT "
#ifndef __${vtk-module}Module_h
#define __${vtk-module}Module_h
${_export_header}
#endif
")
  configure_file(
    ${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}Module.h
    )
  configure_file(
    ${Slicer_CMAKE_DIR}/vtkSlicerObjectFactory.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.h
    )
  configure_file(
    ${Slicer_CMAKE_DIR}/vtkSlicerObjectFactory.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.cxx
    )

  set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.cxx
    PROPERTIES GENERATED 1 WRAP_EXCLUDE 1 ABSTRACT 0
    )

  # add the source file to the source list
  set(${DM_OBJECT_FACTORY_OUTPUT_SRCS_VAR}
    ${${DM_OBJECT_FACTORY_OUTPUT_SRCS_VAR}}
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.cxx
    PARENT_SCOPE
    )

  set(${DM_OBJECT_FACTORY_TARGET_NAME}_AUTOINIT 1 PARENT_SCOPE)
endfunction()
