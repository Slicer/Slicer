
project(SlicerApp)

include(SlicerMacroBuildApplication)

# --------------------------------------------------------------------------
# Properties
# --------------------------------------------------------------------------
SlicerReadApplicationProperties()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_FOLDER "App-${SlicerApp_APPLICATION_NAME}")

# --------------------------------------------------------------------------
# Application library
# --------------------------------------------------------------------------
set(APPLIB_NAME "q${PROJECT_NAME}")

set(APPLIB_SRCS
  qSlicerAppMainWindow.cxx
  qSlicerAppMainWindow.h
  qSlicerAppMainWindow_p.h
  )

set(APPLIB_MOC_SRCS
  qSlicerAppMainWindow.h
  )

set(APPLIB_UI_SRCS
  )

set(APPLIB_RESOURCES
  Resources/SlicerApp.qrc
  ${qSlicerBaseQTGUI_SOURCE_DIR}/Resources/qSlicerBaseQTGUI.qrc
  )

slicerMacroBuildAppLibrary(
  NAME ${APPLIB_NAME}
  APPLICATION_NAME ${SlicerApp_APPLICATION_NAME}
  DESCRIPTION_SUMMARY ${SlicerApp_DESCRIPTION_SUMMARY}
  DESCRIPTION_FILE ${SlicerApp_DESCRIPTION_FILE}
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
  LAUNCHER_SPLASHSCREEN_FILE ${SlicerApp_LAUNCHER_SPLASHSCREEN_FILE}
  APPLE_ICON_FILE ${SlicerApp_APPLE_ICON_FILE}
  WIN_ICON_FILE ${SlicerApp_WIN_ICON_FILE}
  LICENSE_FILE ${SlicerApp_LICENSE_FILE}
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
