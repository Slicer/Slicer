#-----------------------------------------------------------------------------
# Set the default output paths for one or more plugins/CLP
# 
macro(slicer3_set_plugins_output_path)
  set_target_properties(${ARGN}
    PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY 
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    )
endmacro(slicer3_set_plugins_output_path)

#-----------------------------------------------------------------------------
# Install one or more plugins to the default plugin location
# 
macro(slicer3_install_plugins)
  install(TARGETS ${ARGN}
    RUNTIME DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer3_INSTALL_PLUGINS_LIB_DIR} COMPONENT RuntimeLibraries
    )
endmacro(slicer3_install_plugins)

#-----------------------------------------------------------------------------
# Enable plugin testing
#
# function to turn generator name into a version string
# like vs7 vs71 vs8 vs9 
# Lifted from CMake's CTest.cmake
function(GET_VS_VERSION_STRING generator var)
  string(REGEX REPLACE "Visual Studio ([0-9][0-9]?)($|.*)" "\\1" NUMBER "${generator}") 
  if("${generator}" MATCHES "Visual Studio 7 .NET 2003")
    set(ver_string "vs71")
  else("${generator}" MATCHES "Visual Studio 7 .NET 2003")
    set(ver_string "vs${NUMBER}")
  endif("${generator}" MATCHES "Visual Studio 7 .NET 2003")
  set(${var} ${ver_string} PARENT_SCOPE)
endfunction(GET_VS_VERSION_STRING)

macro(slicer3_enable_plugins_testing)

  # We need to use Slicer3's CTest variables so that plugins can submit
  # to the same dashboard. CTest.cmake includes CTestConfig.cmake from
  # PROJECT_SOURCE_DIR but we would like to avoid duplicating this file
  # in each plugins directory, this would make maintenance difficult.
  # Instead, let's just source Slicer3's CTestConfig.cmake and do what
  # CTest.cmake does, only manually.

  if(NOT DEFINED CTEST_DROP_SITE AND NOT EXISTS "${PROJECT_SOURCE_DIR}/CTestConfig.cmake")
    include("${Slicer3_HOME}/${Slicer3_INSTALL_LIB_DIR}/CTestConfig.cmake")
    set(NIGHTLY_START_TIME "${CTEST_NIGHTLY_START_TIME}")
    set(DROP_METHOD "${CTEST_DROP_METHOD}")
    set(DROP_SITE "${CTEST_DROP_SITE}")
    set(DROP_SITE_USER "${CTEST_DROP_SITE_USER}")
    set(DROP_SITE_PASSWORD "${CTEST_DROP_SITE_PASWORD}")
    set(DROP_SITE_MODE "${CTEST_DROP_SITE_MODE}")
    set(DROP_LOCATION "${CTEST_DROP_LOCATION}")
    set(TRIGGER_SITE "${CTEST_TRIGGER_SITE}")
    set(UPDATE_TYPE "${CTEST_UPDATE_TYPE}")
  endif(NOT DEFINED CTEST_DROP_SITE AND NOT EXISTS "${PROJECT_SOURCE_DIR}/CTestConfig.cmake")

  if(NOT DEFINED UPDATE_TYPE)
    set(UPDATE_TYPE svn)
  endif(NOT DEFINED UPDATE_TYPE)

  # CTestConfig.cmake sets the BUILDNAME using some info about the platform
  # and compiler. Since we are submitting to Slicer3 dashboard, we can't
  # just use that, it would be confusing on the dashboard: is it a Slicer3
  # submission or just a plugins submission? Try to help by qualifying the
  # build name with the project name. One way is to set BUILDNAME first, but
  # that would override's CTestConfig.cmake own BUILDNAME which uses info 
  # about platform and OS. Oh well, we need to duplicate code from  
  # CTestConfig.cmake *again* since there is no way to preprend a string.

  if(NOT BUILDNAME)
    set(DART_COMPILER "${CMAKE_CXX_COMPILER}")
    if(NOT DART_COMPILER)
      set(DART_COMPILER "${CMAKE_C_COMPILER}")
    endif(NOT DART_COMPILER)
    if(NOT DART_COMPILER)
      set(DART_COMPILER "unknown")
    endif(NOT DART_COMPILER)
    if(WIN32)
      set(DART_NAME_COMPONENT "NAME_WE")
    else(WIN32)
      set(DART_NAME_COMPONENT "NAME")
    endif(WIN32)
    if(NOT BUILD_NAME_SYSTEM_NAME)
      set(BUILD_NAME_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
      if(WIN32)
        set(BUILD_NAME_SYSTEM_NAME "Win32")
      endif(WIN32)
    endif(NOT BUILD_NAME_SYSTEM_NAME)
    if(UNIX OR BORLAND)
      get_filename_component(DART_CXX_NAME 
        "${CMAKE_CXX_COMPILER}" ${DART_NAME_COMPONENT})
    else(UNIX OR BORLAND)
      get_filename_component(DART_CXX_NAME 
        "${CMAKE_BUILD_TOOL}" ${DART_NAME_COMPONENT})
    endif(UNIX OR BORLAND)
    if(DART_CXX_NAME MATCHES "msdev")
      set(DART_CXX_NAME "vs60")
    endif(DART_CXX_NAME MATCHES "msdev")
    if(DART_CXX_NAME MATCHES "devenv")
      get_vs_version_string("${CMAKE_GENERATOR}" DART_CXX_NAME)
    endif(DART_CXX_NAME MATCHES "devenv")
    set(BUILDNAME "${BUILD_NAME_SYSTEM_NAME}-${DART_CXX_NAME}")
  endif(NOT BUILDNAME)

  set(BUILDNAME "${PROJECT_NAME}-${BUILDNAME}")

  enable_testing()
  include(CTest)

endmacro(slicer3_enable_plugins_testing)

#-----------------------------------------------------------------------------
# Test a plugin
# 
include("${KWWidgets_CMAKE_DIR}/KWWidgetsTestingMacros.cmake")
macro(slicer3_add_plugins_test test_name clp_to_test)
  set(build_type ".")
  if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    # Sadly, there is no way to know if the user picked Debug or Release
    # here, so we are going to have to stick to the value of CMAKE_BUILD_TYPE
    # if it has been set explicitly (by a dashboard for example), or the
    # first value in CMAKE_CONFIGURATION_TYPES (i.e. Debug)/
    kwwidgets_get_cmake_build_type(build_type)
  endif(WIN32 AND CMAKE_CONFIGURATION_TYPES)
  add_test(${test_name} ${Slicer3_DIR}/Slicer3 --launch ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${build_type}/${clp_to_test} ${ARGN}) 
endmacro(slicer3_add_plugins_test)
