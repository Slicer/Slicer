#-----------------------------------------------------------------------------
# Main application
#-----------------------------------------------------------------------------
# Slicer supports more than one application (i.e. an application
# "OtherApp" in addition to "APPLICATION_NAMEApp").
#
# In that specific case, it is required to differentiate the two applications
# and specify which one should be considered as the *Main* one.
#
# This is usually done within the top level CMakeLists.txt file by setting the variable
# Slicer_MAIN_PROJECT.
#
if(NOT DEFINED Slicer_MAIN_PROJECT)
  set(Slicer_MAIN_PROJECT SlicerApp CACHE STRING "Main project name")
endif()
mark_as_superbuild(Slicer_MAIN_PROJECT:STRING)

#-----------------------------------------------------------------------------
# Applications directory
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_APPLICATIONS_DIR)
  set(Slicer_APPLICATIONS_DIR "${Slicer_SOURCE_DIR}/Applications")
endif()
mark_as_superbuild(Slicer_APPLICATIONS_DIR:PATH)

#-----------------------------------------------------------------------------
# Read application properties
#-----------------------------------------------------------------------------
include(SlicerReadApplicationProperties)

SlicerReadApplicationProperties(
  PROJECT_NAME "${Slicer_MAIN_PROJECT}"
  PROPERTIES_VAR application_properties
  )

#-----------------------------------------------------------------------------
# Set Slicer_MAIN_PROJECT_* variables for each application properties
#-----------------------------------------------------------------------------
foreach(property IN LISTS application_properties)
  set(Slicer_MAIN_PROJECT_${property} "${${Slicer_MAIN_PROJECT}_${property}}")
endforeach()

#-----------------------------------------------------------------------------
# Terminal support
#-----------------------------------------------------------------------------
if(WIN32)
  option(Slicer_BUILD_WIN32_CONSOLE "Build ${PROJECT_NAME} executable as a console app on windows (allows debug output)" ON)
else()
  set(Slicer_BUILD_WIN32_CONSOLE OFF)
endif()
mark_as_superbuild(Slicer_BUILD_WIN32_CONSOLE:BOOL)

#-----------------------------------------------------------------------------
# Organization name
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_ORGANIZATION_DOMAIN)
  set(Slicer_ORGANIZATION_DOMAIN "www.na-mic.org" CACHE STRING "Organization domain")
  mark_as_advanced(Slicer_ORGANIZATION_DOMAIN)
endif()
mark_as_superbuild(Slicer_ORGANIZATION_DOMAIN)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} organization domain [${Slicer_ORGANIZATION_DOMAIN}]")

if(NOT DEFINED Slicer_ORGANIZATION_NAME)
  set(Slicer_ORGANIZATION_NAME "NA-MIC" CACHE STRING "Organization name")
  mark_as_advanced(Slicer_ORGANIZATION_NAME)
endif()
mark_as_superbuild(Slicer_ORGANIZATION_NAME)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} organization name [${Slicer_ORGANIZATION_NAME}]")

#-----------------------------------------------------------------------------
# Settings
#-----------------------------------------------------------------------------
set(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME "")
mark_as_superbuild(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME:STRING)

#-----------------------------------------------------------------------------
# Home and favorite modules
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_DEFAULT_HOME_MODULE)
  set(Slicer_DEFAULT_HOME_MODULE "Welcome" CACHE STRING "Name of the module that Slicer activates on start-up by default")
  mark_as_advanced(Slicer_DEFAULT_HOME_MODULE)
endif()
mark_as_superbuild(Slicer_DEFAULT_HOME_MODULE)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} default home module [${Slicer_DEFAULT_HOME_MODULE}]")

if(NOT DEFINED Slicer_DEFAULT_FAVORITE_MODULES)
  set(Slicer_DEFAULT_FAVORITE_MODULES "Data, Volumes, Models, Transforms, Markups, SegmentEditor"
      CACHE STRING "Name of the modules shown on the toolbar by default (comma-separated list)")
  mark_as_advanced(Slicer_DEFAULT_FAVORITE_MODULES)
endif()
mark_as_superbuild(Slicer_DEFAULT_FAVORITE_MODULES)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} default favorite modules [${Slicer_DEFAULT_FAVORITE_MODULES}]")

#-----------------------------------------------------------------------------
# Disclaimer popup
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_DISCLAIMER_AT_STARTUP)
  set(Slicer_DISCLAIMER_AT_STARTUP "Thank you for using %1!<br><br>This software is not intended for clinical use." CACHE STRING "Text displayed at the first startup of Slicer in a popup window")
  # Note: %1 is replaced by the application name and version. Add <br> for newline.
  # If the disclaimer text is set to empty then no disclaimer popup is displayed.
  mark_as_advanced(Slicer_DISCLAIMER_AT_STARTUP)
endif()
mark_as_superbuild(Slicer_DISCLAIMER_AT_STARTUP)
message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} text of disclaimer at startup [${Slicer_DISCLAIMER_AT_STARTUP}]")

#-----------------------------------------------------------------------------
# Set Slicer_MAIN_PROJECT_SOURCE_DIR and <Slicer_MAIN_PROJECT_APPLICATION_NAME>_SOURCE_DIR
#-----------------------------------------------------------------------------
# Propagate source directory to support building Slicer-based application
# that (1) includes Slicer as an external project or (2) add Slicer source
# tree using 'add_subdirectory()'.
# Source directory it then used in 'SlicerConfigureVersionHeaderTarget' module.
if(NOT DEFINED ${Slicer_MAIN_PROJECT_APPLICATION_NAME}_SOURCE_DIR)
  set(${Slicer_MAIN_PROJECT_APPLICATION_NAME}_SOURCE_DIR ${CMAKE_SOURCE_DIR})
endif()
mark_as_superbuild(${Slicer_MAIN_PROJECT_APPLICATION_NAME}_SOURCE_DIR)

#-----------------------------------------------------------------------------
# Set default installation folder and admin account requirement for Windows
#-----------------------------------------------------------------------------
if(WIN32)

  if(NOT DEFINED Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT)
    set(Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT OFF CACHE BOOL
      "Require administrator account to install the application. Must be enabled if Slicer_CPACK_NSIS_INSTALL_ROOT is only writable by administrators.")
    mark_as_advanced(Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT)
  endif()
  mark_as_superbuild(Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT:BOOL)
  message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} requires admin account [${Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT}]")

  if(NOT DEFINED Slicer_CPACK_NSIS_INSTALL_ROOT)

    # Set default install directory (displayed to end user during installation)
    if(Slicer_CPACK_NSIS_INSTALL_REQUIRES_ADMIN_ACCOUNT)
      # User has administrative privileges, therefore we can install to shared folder
      # "C:\Program Files" or "c:\Program Files (x86)".
      if(CMAKE_CL_64)
       set(_install_root_default "$PROGRAMFILES64")
      else()
       set(_install_root_default "$PROGRAMFILES")
      endif()
    else()
      # We do not require administrative privileges, therefore we install to user folder
      # "C:\Users\<username>\AppData\Local".
      set(_install_root_default "$LOCALAPPDATA/${Slicer_ORGANIZATION_NAME}")
    endif()

    set(Slicer_CPACK_NSIS_INSTALL_ROOT ${_install_root_default} CACHE STRING
      "Default installation location. $LOCALAPPDATA, $APPDATA, $PROGRAMFILES, $PROGRAMFILES64 predefined values may be used as basis.")
    mark_as_advanced(Slicer_CPACK_NSIS_INSTALL_ROOT)
  endif()
  mark_as_superbuild(Slicer_CPACK_NSIS_INSTALL_ROOT:STRING)
  message(STATUS "Configuring ${Slicer_MAIN_PROJECT_APPLICATION_NAME} install root [${Slicer_CPACK_NSIS_INSTALL_ROOT}]")

endif()
