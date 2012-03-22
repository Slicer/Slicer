#-----------------------------------------------------------------------------
# Sanity checks
set(expected_defined_vars CMAKE_CTEST_COMMAND EXTENSION_NAME EXTENSION_HOMEPAGE EXTENSION_CATEGORY EXTENSION_SOURCE_DIR EXTENSION_SUPERBUILD_BINARY_DIR EXTENSION_BUILD_SUBDIRECTORY EXTENSION_ENABLED Slicer_CMAKE_DIR Slicer_EXTENSIONS_CMAKE_DIR Slicer_DIR EXTENSION_COMPILER EXTENSION_BITNESS Slicer_EXTENSION_CMAKE_GENERATOR Slicer_WC_REVISION QT_VERSION_MAJOR QT_VERSION_MINOR)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()
set(expected_defined_vars EXTENSION_ICONURL EXTENSION_CONTRIBUTORS EXTENSION_DESCRIPTION EXTENSION_SCREENSHOTURLS)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(AUTHOR_WARNING "Variable ${var} is not defined !")
  endif()
endforeach()
set(expected_existing_vars CMAKE_CTEST_COMMAND Slicer_CMAKE_DIR Slicer_EXTENSIONS_CMAKE_DIR Slicer_DIR)
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

#-----------------------------------------------------------------------------
function(SlicerConvertScriptArgListToCTestFormat script_arg_list output_var)
  # Convert 'script_arg_list' to the format understood by ctest
  foreach(arg ${script_arg_list})
    string(REPLACE "//" "/-/" arg ${arg}) # See http://www.cmake.org/Bug/view.php?id=12953
    string(REPLACE "&" "-AMP-" arg ${arg})
    string(REPLACE "?" "-WHT-" arg ${arg})
    string(REPLACE "(" "-LPAR-" arg ${arg})
    string(REPLACE ")" "-RPAR-" arg ${arg})
    string(REPLACE "=" "\#\#" arg ${arg})
    set(script_args "${script_args}^^${arg}")
  endforeach()
  set(${output_var} ${script_args} PARENT_SCOPE)
endfunction()

#-----------------------------------------------------------------------------
# The following variable can be used while testing the script
#-----------------------------------------------------------------------------
set(CTEST_EXTRA_VERBOSE TRUE)
set(RUN_CTEST_CONFIGURE TRUE)
set(RUN_CTEST_BUILD TRUE)
set(RUN_CTEST_TEST TRUE)
set(RUN_CTEST_PACKAGES TRUE)
set(RUN_CTEST_SUBMIT TRUE)

#-----------------------------------------------------------------------------
# Prepare external project configuration arguments
set(script ${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerBlockBuildPackageAndUploadExtension.cmake)
set(script_arg_list
  CTEST_CMAKE_GENERATOR=${Slicer_EXTENSION_CMAKE_GENERATOR}
  BUILD_TESTING=${BUILD_TESTING}
  RUN_CTEST_CONFIGURE=${RUN_CTEST_CONFIGURE}
  RUN_CTEST_BUILD=${RUN_CTEST_BUILD}
  RUN_CTEST_TEST=${RUN_CTEST_TEST}
  RUN_CTEST_PACKAGES=${RUN_CTEST_PACKAGES}
  RUN_CTEST_SUBMIT=${RUN_CTEST_SUBMIT}
  EXTENSION_BUILD_OPTIONS_STRING=${EXTENSION_BITNESS}bits-Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}
  EXTENSION_COMPILER=${EXTENSION_COMPILER}
  EXTENSION_NAME=${EXTENSION_NAME}
  EXTENSION_CATEGORY=${EXTENSION_CATEGORY}
  EXTENSION_ICONURL=${EXTENSION_ICONURL}
  EXTENSION_CONTRIBUTORS=${EXTENSION_CONTRIBUTORS}
  EXTENSION_DESCRIPTION=${EXTENSION_DESCRIPTION}
  EXTENSION_HOMEPAGE=${EXTENSION_HOMEPAGE}
  EXTENSION_SCREENSHOTURLS=${EXTENSION_SCREENSHOTURLS}
  EXTENSION_SOURCE_DIR=${EXTENSION_SOURCE_DIR}
  EXTENSION_SUPERBUILD_BINARY_DIR=${EXTENSION_SUPERBUILD_BINARY_DIR}
  EXTENSION_BUILD_SUBDIRECTORY=${EXTENSION_BUILD_SUBDIRECTORY}
  EXTENSION_ENABLED=${EXTENSION_ENABLED}
  Slicer_CMAKE_DIR=${Slicer_CMAKE_DIR}
  Slicer_DIR=${Slicer_DIR}
  Slicer_WC_REVISION=${Slicer_WC_REVISION}
  MIDAS_PACKAGE_URL=${MIDAS_PACKAGE_URL}
  MIDAS_PACKAGE_EMAIL=${MIDAS_PACKAGE_EMAIL}
  MIDAS_PACKAGE_API_KEY=${MIDAS_PACKAGE_API_KEY}
  )
if(NOT "${CTEST_MODEL}" STREQUAL "")
  list(APPEND script_arg_list CTEST_MODEL=${CTEST_MODEL})
endif()


#-----------------------------------------------------------------------------
set(CTEST_EXTRA_VERBOSE_ARG "")
if(CTEST_EXTRA_VERBOSE)
  set(CTEST_EXTRA_VERBOSE_ARG "V")
endif()

#-----------------------------------------------------------------------------
# Set CTEST_BUILD_CONFIGURATION here - it shouldn't be escaped using 'SlicerConvertScriptArgListToCTestFormat'
# See http://www.cmake.org/cmake/help/cmake-2-8-docs.html#variable:CMAKE_CFG_INTDIR
if(CMAKE_CONFIGURATION_TYPES)
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_CFG_INTDIR})
else()
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_BUILD_TYPE})
endif()
set(ctest_build_configuration_arg "CTEST_BUILD_CONFIGURATION\#\#${CTEST_BUILD_CONFIGURATION}")

#-----------------------------------------------------------------------------
# Set EXTENSION_TEST_COMMAND
set(script_arg_list_for_test ${script_arg_list})
list(APPEND script_arg_list_for_test RUN_CTEST_UPLOAD=FALSE)
set(script_args "${ctest_build_configuration_arg}")
SlicerConvertScriptArgListToCTestFormat("${script_arg_list_for_test}" script_args)
set(EXTENSION_TEST_COMMAND ${CMAKE_CTEST_COMMAND} -S ${script},${script_args} -V${CTEST_EXTRA_VERBOSE_ARG})

#-----------------------------------------------------------------------------
# Set EXTENSION_UPLOAD_COMMAND
set(script_arg_list_for_upload ${script_arg_list})
list(APPEND script_arg_list_for_upload
  RUN_CTEST_UPLOAD=TRUE
  EXTENSION_ARCHITECTURE=${EXTENSION_ARCHITECTURE}
  EXTENSION_BITNESS=${EXTENSION_BITNESS}
  EXTENSION_OPERATING_SYSTEM=${EXTENSION_OPERATING_SYSTEM}
  )
set(script_args "${ctest_build_configuration_arg}")
SlicerConvertScriptArgListToCTestFormat("${script_arg_list_for_upload}" script_args)
set(EXTENSION_UPLOAD_COMMAND ${CMAKE_CTEST_COMMAND} -C ${CTEST_BUILD_CONFIGURATION} -S ${script},${script_args} -V${CTEST_EXTRA_VERBOSE_ARG})
