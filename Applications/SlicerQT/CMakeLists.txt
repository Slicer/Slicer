project(SlicerQT)

# --------------------------------------------------------------------------
# Build option(s)
# --------------------------------------------------------------------------
set(Slicer_HAS_CONSOLE_IO_SUPPORT TRUE)
if(WIN32)
  option(Slicer_BUILD_WIN32_CONSOLE "Build ${PROJECT_NAME} executable as a console app on windows (allows debug output)" ON)
  set(Slicer_HAS_CONSOLE_IO_SUPPORT ${Slicer_BUILD_WIN32_CONSOLE})
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------

set(SlicerQT_SRCS
  qSlicerAboutDialog.cxx
  qSlicerAboutDialog.h

  qSlicerMainWindow.cxx
  qSlicerMainWindow.h
  qSlicerMainWindowCore.cxx
  qSlicerMainWindowCore.h
  qSlicerMainWindowCore_p.h
  )

set(SlicerQT_MOC_SRCS
  qSlicerAboutDialog.h

  qSlicerMainWindow.h
  qSlicerMainWindowCore.h
  qSlicerMainWindowCore_p.h
  )

set(SlicerQT_UI_SRCS
  Resources/UI/qSlicerAboutDialog.ui
  Resources/UI/qSlicerMainWindow.ui
  )

# Resources
set(qt_module_resources
  Resources/SlicerQT.qrc
  ${qSlicerBaseQTGUI_SOURCE_DIR}/Resources/qSlicerBaseQTGUI.qrc
  ${qSlicerModulesCore_SOURCE_DIR}/Resources/qSlicerModulesCore.qrc
)
#set(SlicerQT_RC_FILE Resources/SlicerQT.rc)


QT4_WRAP_CPP(SlicerQT_SRCS ${SlicerQT_MOC_SRCS})
QT4_WRAP_UI(SlicerQT_UI_CXX ${SlicerQT_UI_SRCS})
QT4_ADD_RESOURCES(SlicerQT_QRC_SRCS ${qt_module_resources})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Slicer_Base_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${qMRMLWidgets_INCLUDE_DIRS}
  ${qSlicerModulesCore_SOURCE_DIR}
  ${qSlicerModulesCore_BINARY_DIR}
  )

include_directories(${include_dirs})


# --------------------------------------------------------------------------
# Build the library
# --------------------------------------------------------------------------

set(KIT_LIBRARY_NAME "q${PROJECT_NAME}")

add_library(${KIT_LIBRARY_NAME}
  ${SlicerQT_SRCS}
  ${SlicerQT_UI_CXX}
  ${SlicerQT_QRC_SRCS}
  )
set_target_properties(${KIT_LIBRARY_NAME} PROPERTIES LABELS ${PROJECT_NAME})

set(SlicerQT_LIBRARIES
  qSlicerModulesCore
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND SlicerQT_LIBRARIES
    qSlicerBaseQTCLI
    )
endif()

target_link_libraries(${KIT_LIBRARY_NAME}
  ${SlicerQT_LIBRARIES}
  )

# --------------------------------------------------------------------------
# Configure Application Bundle Resources (Mac Only)
# --------------------------------------------------------------------------

if(Q_WS_MAC)
  set(apple_icon_file Slicer.icns)
  set(apple_bundle_sources "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${apple_icon_file}")
  set_source_files_properties(
    "${apple_bundle_sources}"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
    )
  set(MACOSX_BUNDLE_ICON_FILE ${apple_icon_file})
endif(Q_WS_MAC)

if(QT_MAC_USE_COCOA)
  get_filename_component(qt_menu_nib
    "@QT_QTGUI_LIBRARY_RELEASE@/Resources/qt_menu.nib"
    REALPATH)

  set(qt_menu_nib_sources
    "${qt_menu_nib}/classes.nib"
    "${qt_menu_nib}/info.nib"
    "${qt_menu_nib}/keyedobjects.nib"
    )
  set_source_files_properties(
    ${qt_menu_nib_sources}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/qt_menu.nib
    )
  else(QT_MAC_USE_COCOA)
    set(qt_menu_nib_sources)
endif(QT_MAC_USE_COCOA)

# --------------------------------------------------------------------------
# Apply user-defined properties to the library target.
# --------------------------------------------------------------------------

if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${KIT_LIBRARY_NAME} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
endif(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${KIT_LIBRARY_NAME}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the executable
# --------------------------------------------------------------------------

if(NOT APPLE)
  set(SlicerQT_EXE_SUFFIX -real)
endif()


set(SlicerQT_EXE_OPTIONS)
if(WIN32)
  if(NOT Slicer_BUILD_WIN32_CONSOLE)
    set(SlicerQT_EXE_OPTIONS WIN32)
  endif()
endif()

if(APPLE)
  set(SlicerQT_EXE_OPTIONS MACOSX_BUNDLE)
endif()

add_executable(${PROJECT_NAME}${SlicerQT_EXE_SUFFIX}
  ${SlicerQT_EXE_OPTIONS}
  Main.cxx
  ${apple_bundle_sources}
  ${qt_menu_nib_sources}
  )
set_target_properties(${PROJECT_NAME}${SlicerQT_EXE_SUFFIX} PROPERTIES LABELS ${PROJECT_NAME})

if(APPLE)
  set_target_properties(${PROJECT_NAME}${SlicerQT_EXE_SUFFIX} PROPERTIES OUTPUT_NAME Slicer)
endif()

target_link_libraries(${PROJECT_NAME}${SlicerQT_EXE_SUFFIX}
  ${KIT_LIBRARY_NAME}
  )

#-----------------------------------------------------------------------------
# Configure
# --------------------------------------------------------------------------

set(MY_LIBRARY_EXPORT_DIRECTIVE "Q_SLICER_APP_EXPORT")
set(MY_EXPORT_HEADER_PREFIX "qSlicerApp")
set(MY_LIBNAME ${KIT_LIBRARY_NAME})

configure_file(
  ${Slicer_SOURCE_DIR}/CMake/qSlicerExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
  )
set(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")

# --------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------

if(NOT APPLE)
  set(SlicerQT_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR})
else()
  set(SlicerQT_INSTALL_DESTINATION_ARGS BUNDLE DESTINATION ".")
endif()

install(TARGETS ${PROJECT_NAME}${SlicerQT_EXE_SUFFIX}
  ${SlicerQT_INSTALL_DESTINATION_ARGS}
  COMPONENT Runtime)

# --------------------------------------------------------------------------
# Configure Slicer Launcher
# --------------------------------------------------------------------------
if(Slicer_USE_CTKAPPLAUNCHER)
  include(${CTKAPPLAUNCHER_DIR}/CMake/ctkAppLauncher.cmake)

  # Define list of extra 'application to launch' to associate with the launcher
  set(extraApplicationToLaunchListForBuildTree)
  if(EXISTS "${QT_DESIGNER_EXECUTABLE}")
    ctkAppLauncherAppendExtraAppToLaunchToList(
      LONG_ARG designer
      HELP "Start Qt designer using Slicer plugins"
      PATH ${QT_DESIGNER_EXECUTABLE}
      OUTPUTVAR extraApplicationToLaunchListForBuildTree
      )
  endif()
  set(executables)
  if(UNIX)
    list(APPEND executables gnome-terminal xterm ddd gdb)
  elseif(WIN32)
    list(APPEND executables VisualStudio cmd)
    set(VisualStudio_EXECUTABLE ${CMAKE_BUILD_TOOL})
    set(cmd_ARGUMENTS "/c start cmd")
  endif()
  foreach(executable ${executables})
    find_program(${executable}_EXECUTABLE ${executable})
    if(${executable}_EXECUTABLE)
      message(STATUS "Enabling Slicer launcher option: --${executable}")
      ctkAppLauncherAppendExtraAppToLaunchToList(
        LONG_ARG ${executable}
        HELP "Start ${executable}"
        PATH ${${executable}_EXECUTABLE}
        ARGUMENTS ${${executable}_ARGUMENTS}
        OUTPUTVAR extraApplicationToLaunchListForBuildTree
        )
    endif()
  endforeach()

  include(SlicerBlockCTKAppLauncherSettings)
  set(splash_image_path ${Slicer_SOURCE_DIR}/Applications/${PROJECT_NAME}/Resources/Images/SlicerSplashScreen.png)
  ctkAppLauncherConfigure(
    TARGET ${PROJECT_NAME}${SlicerQT_EXE_SUFFIX}
    APPLICATION_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
    APPLICATION_NAME Slicer
    SPLASH_IMAGE_PATH ${splash_image_path}
    SPLASH_IMAGE_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
    SPLASHSCREEN_HIDE_DELAY_MS 3000
    HELP_SHORT_ARG "-h"
    HELP_LONG_ARG "--help"
    NOSPLASH_ARGS "--no-splash,--help,--version,--home,--program-path,--no-main-window"
    EXTRA_APPLICATION_TO_LAUNCH_BUILD ${extraApplicationToLaunchListForBuildTree}
    DESTINATION_DIR ${Slicer_BINARY_DIR}
    LIBRARY_PATHS_BUILD "${SLICER_LIBRARY_PATHS_BUILD}"
    PATHS_BUILD "${SLICER_PATHS_BUILD}"
    ENVVARS_BUILD "${SLICER_ENVVARS_BUILD}"
    LIBRARY_PATHS_INSTALLED "${SLICER_LIBRARY_PATHS_INSTALLED}"
    PATHS_INSTALLED "${SLICER_PATHS_INSTALLED}"
    ENVVARS_INSTALLED "${SLICER_ENVVARS_INSTALLED}"
    )
  # HACK - Waiting "ctkAppLauncherConfigure" macro is updated so that it reconfigures the settings,
  #        if possible, only if "SlicerBlockCTKAppLauncherSettings.cmake" has been modified.
  #        Let's force a systematic reconfiguration of SlicerLauncherSettings.
  add_custom_target(SlicerRemoveLauncherSettings
    COMMAND ${CMAKE_COMMAND} -E remove
      ${Slicer_BINARY_DIR}/SlicerLauncherSettings.ini
      ${Slicer_BINARY_DIR}/SlicerLauncherSettingsToInstall.ini
    )
  add_dependencies(SlicerRemoveLauncherSettings ${PROJECT_NAME}${SlicerQT_EXE_SUFFIX})
  add_dependencies(SlicerConfigureLauncher SlicerRemoveLauncherSettings)

  if(NOT APPLE)
    if(Slicer_HAS_CONSOLE_IO_SUPPORT)
      install(PROGRAMS "${Slicer_BINARY_DIR}/Slicer${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION ".")
    else()
      install(
        PROGRAMS "${CTKAPPLAUNCHER_DIR}/bin/CTKAppLauncherW${CMAKE_EXECUTABLE_SUFFIX}"
        DESTINATION "."
        RENAME "Slicer${CMAKE_EXECUTABLE_SUFFIX}"
        )
    endif()

    install(
      FILES ${splash_image_path}
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      )
    install(
      FILES "${Slicer_BINARY_DIR}/SlicerLauncherSettingsToInstall.ini"
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      RENAME SlicerLauncherSettings.ini
      )
  endif()
endif()

# --------------------------------------------------------------------------
# Testing
#
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
