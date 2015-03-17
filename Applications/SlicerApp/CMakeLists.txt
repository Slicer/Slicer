project(SlicerApp)

include(SlicerMacroBuildApplication)

# --------------------------------------------------------------------------
# Slicer supports more than one application (i.e. an application
# "OtherApp" in addition to "APPLICATION_NAMEApp").
#
# In that specific case, it is required to differentiate the two applications
# and specify which one should be considered as the *Main* one.
#
# This is usually done within the top level CMakeLists.txt file by setting the variable
# Slicer_MAIN_PROJECT.
#
# Within the Applications/OtherApp/CMakeLists.txt of the secondary application,
# the project name should be updated, the variable 'SlicerApp_APPLICATION_NAME'
# could also be optionnally set to a custom value.
#

# set(SlicerApp_APPLICATION_NAME ${PROJECT_NAME}) # Uncomment and optionnally set this variable using a custom value.

if(${PROJECT_NAME} STREQUAL ${Slicer_MAIN_PROJECT})
  set(SlicerApp_APPLICATION_NAME ${Slicer_MAIN_PROJECT_APPLICATION_NAME})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_FOLDER "App-${SlicerApp_APPLICATION_NAME}")

# --------------------------------------------------------------------------
# Application library
# --------------------------------------------------------------------------

set(APPLIB_NAME "q${PROJECT_NAME}")

set(APPLIB_DESCRIPTION_SUMMARY
  "Medical Visualization and Processing Environment for Research"
  )

set(APPLIB_SRCS
  qSlicerAppAboutDialog.cxx
  qSlicerAppAboutDialog.h
  qSlicerAppErrorReportDialog.cxx
  qSlicerAppErrorReportDialog.h
  qSlicerAppMainWindow.cxx
  qSlicerAppMainWindow.h
  qSlicerAppMainWindow_p.h
  )

set(APPLIB_MOC_SRCS
  qSlicerAppAboutDialog.h
  qSlicerAppErrorReportDialog.h
  qSlicerAppMainWindow.h
  )

set(APPLIB_UI_SRCS
  Resources/UI/qSlicerAppAboutDialog.ui
  Resources/UI/qSlicerAppErrorReportDialog.ui
  Resources/UI/qSlicerAppMainWindow.ui
  )

set(APPLIB_RESOURCES
  Resources/SlicerApp.qrc
  ${qSlicerBaseQTGUI_SOURCE_DIR}/Resources/qSlicerBaseQTGUI.qrc
  )

slicerMacroBuildAppLibrary(
  NAME ${APPLIB_NAME}
  APPLICATION_NAME ${SlicerApp_APPLICATION_NAME}
  DESCRIPTION_SUMMARY ${APPLIB_DESCRIPTION_SUMMARY}
  DESCRIPTION_FILE ${Slicer_SOURCE_DIR}/README.txt
  EXPORT_DIRECTIVE "Q_SLICER_APP_EXPORT"
  FOLDER ${${PROJECT_NAME}_FOLDER}
  SRCS ${APPLIB_SRCS}
  MOC_SRCS ${APPLIB_MOC_SRCS}
  UI_SRCS ${APPLIB_UI_SRCS}
  RESOURCES ${APPLIB_RESOURCES}
  )

# --------------------------------------------------------------------------
# Application executable
# --------------------------------------------------------------------------

# Configure launcher only for the main application
set(extra_args)
if(${PROJECT_NAME} STREQUAL ${Slicer_MAIN_PROJECT})
  set(extra_args CONFIGURE_LAUNCHER)
endif()

set(APP_SRCS
  Main.cxx
  )

slicerMacroBuildApplication(
  NAME ${PROJECT_NAME}
  APPLICATION_NAME ${${PROJECT_NAME}_APPLICATION_NAME}
  FOLDER ${${PROJECT_NAME}_FOLDER}
  SRCS ${APP_SRCS}
  TARGET_LIBRARIES ${APPLIB_NAME}
  TARGET_NAME_VAR "APP_TARGET_NAME"
  ${extra_args}
  )

# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
