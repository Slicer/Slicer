
#
# This module will add a target named 'SlicerGenerateExtensionTemplates'.
#
# It has been designed to be included in the build system of Slicer.
#

#SuperBuildExtensionTemplate.s4ext
if(NOT DEFINED GENERATE_EXTENSION_DESCRIPTION_FILE)
  set(GENERATE_EXTENSION_DESCRIPTION_FILE 0)
endif()

#-----------------------------------------------------------------------------
# Generate extension description file

if(GENERATE_EXTENSION_DESCRIPTION_FILE)

  # Sanity checks
  set(expected_defined_vars
    DESCRIPTION_FILE
    LOCAL_EXTENSIONS_DIR
    EXTENSION_RELATIVE_DIR
    Slicer_BINARY_DIR
    )
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} is not defined !")
    endif()
  endforeach()

  # Generate description file
  execute_process(
    COMMAND ${Slicer_BINARY_DIR}/bin/slicerExtensionWizard
      --localExtensionsDir=${LOCAL_EXTENSIONS_DIR}
      --describe ${LOCAL_EXTENSIONS_DIR}/${EXTENSION_RELATIVE_DIR}
    WORKING_DIRECTORY ${LOCAL_EXTENSIONS_DIR}
    OUTPUT_VARIABLE content
  )
  file(WRITE ${DESCRIPTION_FILE} ${content})

  return()
endif()

if(NOT UNIX)
  return()
endif()

#-----------------------------------------------------------------------------
# Add 'SlicerGenerateExtensionTemplates' target

set(_template_dir ${Slicer_SOURCE_DIR}/Extensions)
set(_extension_template_generator_commands )

# Add custom command re-generating an extension template
macro(_append_extension_template_generator_commands)
  set(options)
  set(oneValueArgs
    EXTENSION_NAME
    EXTENSION_TYPE
    )
  set(multiValueArgs
    MODULE_TYPES
    )
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if(NOT MY_MODULE_TYPES)
    message(FATAL_ERROR "Parameter MODULE_TYPES is not specified !")
  endif()

  # Set defaults
  if(NOT MY_EXTENSION_NAME)
    list(GET MY_MODULE_TYPES 0 extension_name_prefix)
    if("${extension_name_prefix}" STREQUAL "ScriptedEditorEffect")
      set(extension_name_prefix "Editor")
    endif()
    set(MY_EXTENSION_NAME "${extension_name_prefix}ExtensionTemplate")
  endif()
  if(NOT MY_EXTENSION_TYPE)
    set(MY_EXTENSION_TYPE "default")
  endif()

  set(extension_name ${MY_EXTENSION_NAME})
  set(extension_type ${MY_EXTENSION_TYPE})
  set(extension_dir ${_template_dir}/Testing/${extension_name})
  set(description_file ${_template_dir}/${extension_name}.s4ext)

  set(wizard_add_module_args)
  set(additional_wizard_args)

  foreach(module_type IN LISTS MY_MODULE_TYPES)

    set(wizard_module_type ${module_type})
    if("${module_type}" STREQUAL "ScriptedLoadable")
      set(wizard_module_type "Scripted")
    endif()

    set(module_name "${module_type}ModuleTemplate")
    if(extension_type STREQUAL "superbuild")
      set(module_name "Super${module_name}")
    endif()

    if("${module_type}" STREQUAL "ScriptedEditorEffect")
      set(module_name "EditorEffectTemplate")
      list(APPEND additional_wizard_args
        --templateKey ${module_type}=TemplateKeyEffect
        )
    endif()

    list(APPEND wizard_add_module_args
        --addModule ${wizard_module_type}:${module_name}
      )
  endforeach()

  list(APPEND _extension_template_generator_commands
    COMMAND ${CMAKE_COMMAND} -E remove -f ${description_file}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${extension_dir}
    COMMAND ${Slicer_BINARY_DIR}/bin/slicerExtensionWizard
      ${wizard_add_module_args}
      --create ${extension_type}:${extension_name}
      ${additional_wizard_args}
      ${_template_dir}/Testing
    COMMAND ${CMAKE_COMMAND}
      -DDESCRIPTION_FILE:FILEPATH=${description_file}
      -DLOCAL_EXTENSIONS_DIR:PATH=${_template_dir}
      -DEXTENSION_RELATIVE_DIR:PATH=Testing/${extension_name}
      -DSlicer_BINARY_DIR:PATH=${Slicer_BINARY_DIR}
      -DGENERATE_EXTENSION_DESCRIPTION_FILE:BOOL=1
      -P ${CMAKE_CURRENT_LIST_FILE}
    )
endmacro()

# Loop over module types and add template generator commands
foreach(type IN ITEMS
    CLI
    Loadable
    Scripted
    ScriptedCLI
    ScriptedEditorEffect
    ScriptedSegmentEditorEffect
    )
  _append_extension_template_generator_commands(MODULE_TYPES ${type})
endforeach()

# Add template generator command for a SuperBuild extension
_append_extension_template_generator_commands(
  EXTENSION_NAME "SuperBuildExtensionTemplate"
  EXTENSION_TYPE "superbuild"
  MODULE_TYPES
    CLI
    Loadable
  )

# Add convenience target allowing to re-generate templates
add_custom_target(SlicerGenerateExtensionTemplates
  ${_extension_template_generator_commands}
  WORKING_DIRECTORY ${_template_dir}
  COMMENT "Generating extension testing templates"
  VERBATIM
  )
