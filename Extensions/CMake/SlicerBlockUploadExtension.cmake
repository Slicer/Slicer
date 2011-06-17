
# Sanity checks
set(expected_defined_vars EXTENSION_NAME EXTENSION_SOURCE_DIR EXTENSION_BINARY_DIR Slicer_CMAKE_DIR Slicer_EXTENSIONS_CMAKE_DIR Slicer_DIR EXTENSION_COMPILER EXTENSION_BITNESS Slicer_EXTENSION_CMAKE_GENERATOR Slicer_WC_REVISION QT_VERSION_MAJOR QT_VERSION_MINOR)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var}) 
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

FUNCTION(SlicerConvertScriptArgListToCTestFormat script_arg_list output_var)
  # Convert 'script_arg_list' to the format understood by ctest
  FOREACH(arg ${script_arg_list})
    STRING(REPLACE "=" "\#\#" arg ${arg})
    SET(script_args "${script_args}^^${arg}")
  ENDFOREACH()
  SET(${output_var} ${script_args} PARENT_SCOPE)
ENDFUNCTION()

# Prepare external project configuration arguments
SET(script ${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerBlockBuildPackageAndUploadExtension.cmake)
SET(script_arg_list
  CTEST_CMAKE_GENERATOR=${Slicer_EXTENSION_CMAKE_GENERATOR}
  RUN_CTEST_SUBMIT=TRUE # Could be set to FALSE for debug purposes
  EXTENSION_BUILD_OPTIONS_STRING=${EXTENSION_BITNESS}bits-Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}
  EXTENSION_COMPILER=${EXTENSION_COMPILER}
  EXTENSION_NAME=${EXTENSION_NAME}
  EXTENSION_SOURCE_DIR=${EXTENSION_SOURCE_DIR}
  EXTENSION_BINARY_DIR=${EXTENSION_BINARY_DIR}
  Slicer_CMAKE_DIR=${Slicer_CMAKE_DIR}
  Slicer_DIR=${Slicer_DIR}
  Slicer_WC_REVISION=${Slicer_WC_REVISION}
  )
IF(NOT "${CMAKE_CFG_INTDIR}" STREQUAL "")
  LIST(APPEND script_arg_list CMAKE_CFG_INTDIR=${CMAKE_CFG_INTDIR})
ENDIF()
IF(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "")
  LIST(APPEND script_arg_list CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
ENDIF()
IF(NOT "${CTEST_MODEL}" STREQUAL "")
  LIST(APPEND script_arg_list CTEST_MODEL=${CTEST_MODEL})
ENDIF()

SET(script_arg_list_for_test ${script_arg_list})
LIST(APPEND script_arg_list_for_test RUN_CTEST_UPLOAD=FALSE)
SET(script_args "")
SlicerConvertScriptArgListToCTestFormat("${script_arg_list_for_test}" script_args)
SET(EXTENSION_TEST_COMMAND ${CMAKE_CTEST_COMMAND} -S ${script},${script_args} -V)

SET(script_arg_list_for_upload ${script_arg_list})
LIST(APPEND script_arg_list_for_upload RUN_CTEST_UPLOAD=TRUE)
SET(script_args "")
SlicerConvertScriptArgListToCTestFormat("${script_arg_list_for_upload}" script_args)
SET(EXTENSION_UPLOAD_COMMAND ${CMAKE_CTEST_COMMAND} -S ${script},${script_args} -V)
