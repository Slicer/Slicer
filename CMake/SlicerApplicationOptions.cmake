#-----------------------------------------------------------------------------
# Main application
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_MAIN_PROJECT)
  set(Slicer_MAIN_PROJECT SlicerApp CACHE INTERNAL "Main project name")
endif()
mark_as_superbuild(Slicer_MAIN_PROJECT:STRING)
if(NOT DEFINED ${Slicer_MAIN_PROJECT}_APPLICATION_NAME)
  set(${Slicer_MAIN_PROJECT}_APPLICATION_NAME Slicer CACHE INTERNAL "Main application name")
else()
  if(NOT DEFINED SlicerApp_APPLICATION_NAME)
    set(SlicerApp_APPLICATION_NAME Slicer)
  endif()
endif()

set(Slicer_MAIN_PROJECT_APPLICATION_NAME ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})

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
  set(Slicer_DEFAULT_FAVORITE_MODULES "Data, Volumes, Models, Transforms, Markups, Editor, SegmentEditor"
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
# Applications directory
#-----------------------------------------------------------------------------
if(NOT DEFINED Slicer_APPLICATIONS_DIR)
  set(Slicer_APPLICATIONS_DIR "Applications")
endif()
mark_as_superbuild(Slicer_APPLICATIONS_DIR:PATH)
