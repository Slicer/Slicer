#-----------------------------------------------------------------------------
# Sanity checks
set(expected_defined_vars
  CMAKE_CTEST_COMMAND
  EXTENSION_BITNESS
  EXTENSION_BUILD_SUBDIRECTORY
  EXTENSION_COMPILER
  EXTENSION_ENABLED
  EXTENSION_NAME
  EXTENSION_SOURCE_DIR
  EXTENSION_SUPERBUILD_BINARY_DIR
  GIT_EXECUTABLE
  Slicer_CMAKE_DIR
  Slicer_DIR
  Slicer_EXTENSION_CMAKE_GENERATOR
  Slicer_EXTENSIONS_CMAKE_DIR
  Slicer_QT_VERSION_MAJOR
  Slicer_QT_VERSION_MINOR
  Slicer_REVISION
  Subversion_SVN_EXECUTABLE
  )
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

set(expected_existing_vars
  CMAKE_CTEST_COMMAND
  GIT_EXECUTABLE
  Slicer_CMAKE_DIR
  Slicer_DIR
  Slicer_EXTENSIONS_CMAKE_DIR
  # Since new extension generated from the SuperBuild template
  # do not require SVN, we do not require it.
  # Subversion_SVN_EXECUTABLE
  )
foreach(var ${expected_existing_vars})
  if(NOT EXISTS "${${var}}")
    message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Macro allowing to set a variable to its default value if not already defined.
# If defined, the default value is set with the value of environment variable <var>.
# Otherwise, it is set with the value passed as a parameter.
macro(_set_if_env_not_defined var defaultvalue)
  if(DEFINED ENV{${var}})
    set(_value "$ENV{${var}}")
    message(STATUS "Setting '${var}' variable with environment variable value '${_value}'")
    set(${var} $ENV{${var}})
  else()
    set(_value "${defaultvalue}")
    message(STATUS "Setting '${var}' variable with default value '${_value}'")
    set(${var} "${defaultvalue}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# The following variable can be used while testing the script
#-----------------------------------------------------------------------------
set(CTEST_EXTRA_VERBOSE TRUE)
_set_if_env_not_defined(run_extension_ctest_with_configure TRUE)
_set_if_env_not_defined(run_extension_ctest_with_build TRUE)
_set_if_env_not_defined(run_extension_ctest_with_test TRUE)
_set_if_env_not_defined(run_extension_ctest_with_packages TRUE)
_set_if_env_not_defined(run_extension_ctest_submit TRUE)
set(RUN_CTEST_CONFIGURE ${run_extension_ctest_with_configure})
set(RUN_CTEST_BUILD ${run_extension_ctest_with_build})
set(RUN_CTEST_TEST ${run_extension_ctest_with_test})
set(RUN_CTEST_PACKAGES ${run_extension_ctest_with_packages})
set(RUN_CTEST_SUBMIT ${run_extension_ctest_submit})

#-----------------------------------------------------------------------------
# Prepare external project configuration arguments
set(EXTENSION_SCRIPT ${Slicer_EXTENSIONS_CMAKE_DIR}/SlicerBlockBuildPackageAndUploadExtension.cmake)
set(EXTENSION_COMMAND_ARG_LIST
"set(CTEST_CMAKE_GENERATOR \"${Slicer_EXTENSION_CMAKE_GENERATOR}\")
set(GIT_EXECUTABLE \"${GIT_EXECUTABLE}\")
set(Subversion_SVN_EXECUTABLE \"${Subversion_SVN_EXECUTABLE}\")
set(CMAKE_MAKE_PROGRAM \"${CMAKE_MAKE_PROGRAM}\")
set(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")
set(CMAKE_CXX_COMPILER \"${CMAKE_CXX_COMPILER}\")
set(CMAKE_CXX_STANDARD \"${CMAKE_CXX_STANDARD}\")
set(CMAKE_CXX_STANDARD_REQUIRED \"${CMAKE_CXX_STANDARD_REQUIRED}\")
set(CMAKE_CXX_EXTENSIONS \"${CMAKE_CXX_EXTENSIONS}\")
set(BUILD_TESTING \"${BUILD_TESTING}\")
set(RUN_CTEST_SUBMIT \"${RUN_CTEST_SUBMIT}\")
set(CTEST_SITE \"${CTEST_SITE}\")
set(CDASH_PROJECT_NAME \"${CDASH_PROJECT_NAME}\")
set(EXTENSION_BUILD_OPTIONS_STRING \"${EXTENSION_BITNESS}bits-Qt${Slicer_QT_VERSION_MAJOR}.${Slicer_QT_VERSION_MINOR}\")
set(EXTENSION_COMPILER \"${EXTENSION_COMPILER}\")
set(EXTENSION_NAME \"${EXTENSION_NAME}\")
set(EXTENSION_CATEGORY \"${EXTENSION_CATEGORY}\")
set(EXTENSION_STATUS \"${EXTENSION_STATUS}\")
set(EXTENSION_ICONURL \"${EXTENSION_ICONURL}\")
set(EXTENSION_CONTRIBUTORS \"${EXTENSION_CONTRIBUTORS}\")
set(EXTENSION_DESCRIPTION \"${EXTENSION_DESCRIPTION}\")
set(EXTENSION_HOMEPAGE \"${EXTENSION_HOMEPAGE}\")
set(EXTENSION_SCREENSHOTURLS \"${EXTENSION_SCREENSHOTURLS}\")
set(EXTENSION_SOURCE_DIR \"${EXTENSION_SOURCE_DIR}\")
set(EXTENSION_SUPERBUILD_BINARY_DIR \"${EXTENSION_SUPERBUILD_BINARY_DIR}\")
set(EXTENSION_BUILD_SUBDIRECTORY \"${EXTENSION_BUILD_SUBDIRECTORY}\")
set(EXTENSION_ENABLED \"${EXTENSION_ENABLED}\")
set(EXTENSION_DEPENDS \"${EXTENSION_DEPENDS}\")
set(Slicer_CMAKE_DIR \"${Slicer_CMAKE_DIR}\")
set(Slicer_DIR \"${Slicer_DIR}\")
set(Slicer_EXTENSIONS_TRACK_QUALIFIER \"${Slicer_EXTENSIONS_TRACK_QUALIFIER}\")
set(Slicer_REVISION \"${Slicer_REVISION}\")
set(MIDAS_PACKAGE_URL \"${MIDAS_PACKAGE_URL}\")
set(MIDAS_PACKAGE_EMAIL \"${MIDAS_PACKAGE_EMAIL}\")
set(MIDAS_PACKAGE_API_KEY \"${MIDAS_PACKAGE_API_KEY}\")")
if(APPLE)
  set(EXTENSION_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(CMAKE_OSX_ARCHITECTURES \"${CMAKE_OSX_ARCHITECTURES}\")
set(CMAKE_OSX_DEPLOYMENT_TARGET \"${CMAKE_OSX_DEPLOYMENT_TARGET}\")
set(CMAKE_OSX_SYSROOT \"${CMAKE_OSX_SYSROOT}\")")
endif()
if(DEFINED CMAKE_JOB_POOLS)
  set(EXTENSION_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(CMAKE_JOB_POOLS \"${CMAKE_JOB_POOLS}\")
set(CMAKE_JOB_POOL_COMPILE \"${CMAKE_JOB_POOL_COMPILE}\")
set(CMAKE_JOB_POOL_LINK \"${CMAKE_JOB_POOL_LINK}\")")
endif()
foreach(dep ${EXTENSION_DEPENDS})
  set(EXTENSION_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(${dep}_BINARY_DIR \"${${dep}_BINARY_DIR}\")
set(${dep}_BUILD_SUBDIRECTORY \"${${dep}_BUILD_SUBDIRECTORY}\")
")
endforeach()

#-----------------------------------------------------------------------------
set(CTEST_EXTRA_VERBOSE_ARG "")
if(CTEST_EXTRA_VERBOSE)
  set(CTEST_EXTRA_VERBOSE_ARG "V")
endif()

#-----------------------------------------------------------------------------
# Set CTEST_BUILD_CONFIGURATION here
# See http://www.cmake.org/cmake/help/cmake-2-8-docs.html#variable:CMAKE_CFG_INTDIR
if(CMAKE_CONFIGURATION_TYPES)
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_CFG_INTDIR})
else()
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_BUILD_TYPE})
endif()
set(EXTENSION_COMMAND_BUILD_CONF_ARG_LIST
  -C ${CTEST_BUILD_CONFIGURATION}
  -DCTEST_BUILD_CONFIGURATION:STRING=${CTEST_BUILD_CONFIGURATION}
  )
set(dollar "$")
set(EXTENSION_COMMAND_BUILD_CONF_WRAPPER_ARG_LIST
  -C ${dollar}{CTEST_BUILD_CONFIGURATION}
  -DCTEST_BUILD_CONFIGURATION:STRING=${dollar}{CTEST_BUILD_CONFIGURATION}
  )

#-----------------------------------------------------------------------------
# Set EXTENSION_TEST_COMMAND
set(EXTENSION_TEST_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(CTEST_DROP_SITE \"${CTEST_DROP_SITE}\")
set(RUN_CTEST_CONFIGURE \"${RUN_CTEST_CONFIGURE}\")
set(RUN_CTEST_BUILD \"${RUN_CTEST_BUILD}\")
set(RUN_CTEST_TEST \"${RUN_CTEST_TEST}\")
set(RUN_CTEST_PACKAGES \"${RUN_CTEST_PACKAGES}\")
set(RUN_CTEST_UPLOAD \"FALSE\")")

set(script_args_file ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-test-command-args.cmake)
file(WRITE ${script_args_file} "${EXTENSION_TEST_COMMAND_ARG_LIST}")

set(EXTENSION_TEST_COMMAND_ARGS -DCTEST_MODEL:STRING=${CTEST_MODEL} -DSCRIPT_ARGS_FILE:FILEPATH=${script_args_file} -S ${EXTENSION_SCRIPT} -V${CTEST_EXTRA_VERBOSE_ARG})
set(EXTENSION_TEST_WRAPPER_COMMAND ${CMAKE_CTEST_COMMAND} ${EXTENSION_COMMAND_BUILD_CONF_WRAPPER_ARG_LIST} ${EXTENSION_TEST_COMMAND_ARGS})

#-----------------------------------------------------------------------------
# Set EXTENSION_UPLOAD_COMMAND
set(EXTENSION_UPLOAD_COMMAND_ARG_LIST "${EXTENSION_COMMAND_ARG_LIST}
set(CTEST_DROP_SITE \"${CTEST_DROP_SITE}\")
set(RUN_CTEST_CONFIGURE \"${RUN_CTEST_CONFIGURE}\")
set(RUN_CTEST_BUILD \"${RUN_CTEST_BUILD}\")
set(RUN_CTEST_TEST \"${RUN_CTEST_TEST}\")
set(RUN_CTEST_PACKAGES \"${RUN_CTEST_PACKAGES}\")
set(RUN_CTEST_UPLOAD \"TRUE\")
set(EXTENSION_ARCHITECTURE \"${EXTENSION_ARCHITECTURE}\")
set(EXTENSION_BITNESS \"${EXTENSION_BITNESS}\")
set(EXTENSION_OPERATING_SYSTEM \"${EXTENSION_OPERATING_SYSTEM}\")")

set(script_args_file ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}-upload-command-args.cmake)
file(WRITE ${script_args_file} "${EXTENSION_UPLOAD_COMMAND_ARG_LIST}")

set(EXTENSION_UPLOAD_COMMAND_ARGS -DCTEST_MODEL:STRING=${CTEST_MODEL} -DSCRIPT_ARGS_FILE:FILEPATH=${script_args_file} -S ${EXTENSION_SCRIPT} -V${CTEST_EXTRA_VERBOSE_ARG})
set(EXTENSION_UPLOAD_WRAPPER_COMMAND ${CMAKE_CTEST_COMMAND} ${EXTENSION_COMMAND_BUILD_CONF_WRAPPER_ARG_LIST} ${EXTENSION_UPLOAD_COMMAND_ARGS})
