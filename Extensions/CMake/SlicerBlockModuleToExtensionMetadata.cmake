
# Sanity checks
set(var EXTENSION_HAS_ONLY_ONE_MODULE)
if(NOT DEFINED ${var})
  message(FATAL_ERROR "error: ${var} is either NOT defined or empty.")
endif()

#
# If EXTENSION_HAS_ONLY_ONE_MODULE is TRUE, metadata should NOT be specified using EXTENSION_* variables.
#
set(_metatdata_list HOMEPAGE CATEGORY ICONURL STATUS CONTRIBUTORS DESCRIPTION
                    SCREENSHOTURLS ACKNOWLEDGEMENTS
                    MAJOR_VERSION MINOR_VERSION PATCH_VERSION
                    DEPENDS
                    LICENSE_FILE LICENSE_SHORT_DESCRIPTION README_FILE
                    )
foreach(varname ${_metatdata_list})
  if(${EXTENSION_HAS_ONLY_ONE_MODULE} AND NOT "${EXTENSION_${varname}_CONVERTED}" STREQUAL "DONE")
    if(DEFINED EXTENSION_${varname})
      message(FATAL_ERROR "Since EXTENSION_HAS_ONLY_ONE_MODULE is ${EXTENSION_HAS_ONLY_ONE_MODULE}, "
                          "metadata associated with this extension are expected to be described using "
                          "'MODULE_*' variables.\n"
                          "Variable 'EXTENSION_${varname}' should probably be changed into 'MODULE_${varname}'\n")
    endif()
    if(DEFINED MODULE_${varname})
      set(EXTENSION_${varname} ${MODULE_${varname}})
      set(EXTENSION_${varname}_CONVERTED "DONE")
      slicer_setting_variable_message("EXTENSION_${varname}")
    endif()
  endif()
endforeach()
