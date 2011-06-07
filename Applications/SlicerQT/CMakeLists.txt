PROJECT(SlicerQT)

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------

SET(SlicerQT_SRCS
  qSlicerAboutDialog.cxx
  qSlicerAboutDialog.h

  qSlicerMainWindow.cxx
  qSlicerMainWindow.h
  qSlicerMainWindowCore.cxx
  qSlicerMainWindowCore.h
  qSlicerMainWindowCore_p.h
  )

SET(SlicerQT_MOC_SRCS
  qSlicerAboutDialog.h

  qSlicerMainWindow.h
  qSlicerMainWindowCore.h
  qSlicerMainWindowCore_p.h
  )
  
SET(SlicerQT_UI_SRCS
  Resources/UI/qSlicerAboutDialog.ui
  Resources/UI/qSlicerMainWindow.ui
  )

# Resources
SET(qt_module_resources
  Resources/SlicerQT.qrc
  ${qSlicerBaseQTGUI_SOURCE_DIR}/Resources/qSlicerBaseQTGUI.qrc
  ${qSlicerBaseQTCoreModules_SOURCE_DIR}/Resources/qSlicerBaseQTCoreModules.qrc
)
#SET(SlicerQT_RC_FILE Resources/SlicerQT.rc)


QT4_WRAP_CPP(SlicerQT_SRCS ${SlicerQT_MOC_SRCS})
QT4_WRAP_UI(SlicerQT_UI_CXX ${SlicerQT_UI_SRCS})
QT4_ADD_RESOURCES(SlicerQT_QRC_SRCS ${qt_module_resources})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  #${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

INCLUDE_DIRECTORIES(${include_dirs})


# --------------------------------------------------------------------------
# Build the library
# --------------------------------------------------------------------------

SET(lib_name "qSlicerQT")

ADD_LIBRARY(${lib_name}
  ${SlicerQT_SRCS}
  ${SlicerQT_UI_CXX}
  ${SlicerQT_QRC_SRCS}
  )
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES LABELS SlicerQT)

SET(SlicerQT_LIBRARIES
  qSlicerBaseQTCLI
  qSlicerBaseQTCoreModules
  )

IF(Slicer_USE_PYTHONQT)
  SET(SlicerQT_LIBRARIES 
    ${SlicerQT_LIBRARIES}
    qSlicerBaseQTCorePythonQt
    qSlicerBaseQTGUIPythonQt
    )
ENDIF()

target_link_libraries(${lib_name} 
  ${SlicerQT_LIBRARIES}
  )


# --------------------------------------------------------------------------
# Configure Application Bundle Resources (Mac Only)
# --------------------------------------------------------------------------

IF(Q_WS_MAC)
  SET(apple_icon_file Slicer.icns)
  SET(apple_bundle_sources "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${apple_icon_file}")
  SET_SOURCE_FILES_PROPERTIES(
    "${apple_bundle_sources}"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
    )
  SET(MACOSX_BUNDLE_ICON_FILE ${apple_icon_file})
ENDIF(Q_WS_MAC)

IF(QT_MAC_USE_COCOA)
  GET_FILENAME_COMPONENT(qt_menu_nib
    "@QT_QTGUI_LIBRARY_RELEASE@/Resources/qt_menu.nib"
    REALPATH)

  SET(qt_menu_nib_sources
    "${qt_menu_nib}/classes.nib"
    "${qt_menu_nib}/info.nib"
    "${qt_menu_nib}/keyedobjects.nib"
    )
  SET_SOURCE_FILES_PROPERTIES(
    ${qt_menu_nib_sources}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/qt_menu.nib
    )
  ELSE(QT_MAC_USE_COCOA)
    set(qt_menu_nib_sources)
ENDIF(QT_MAC_USE_COCOA)

# --------------------------------------------------------------------------
# Apply user-defined properties to the library target.
# --------------------------------------------------------------------------

IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Build the executable
# --------------------------------------------------------------------------

IF(NOT APPLE)
  SET(SlicerQT_EXE_SUFFIX -real)
ENDIF()

ADD_EXECUTABLE(SlicerQT${SlicerQT_EXE_SUFFIX} MACOSX_BUNDLE
  Main.cxx
  ${apple_bundle_sources}
  ${qt_menu_nib_sources}
  )
SET_TARGET_PROPERTIES(SlicerQT${SlicerQT_EXE_SUFFIX} PROPERTIES LABELS SlicerQT)

IF(APPLE)
  SET_TARGET_PROPERTIES(SlicerQT${SlicerQT_EXE_SUFFIX} PROPERTIES OUTPUT_NAME Slicer)
ENDIF()

TARGET_LINK_LIBRARIES(SlicerQT${SlicerQT_EXE_SUFFIX}
  ${lib_name}
  )

#-----------------------------------------------------------------------------
# Configure
# --------------------------------------------------------------------------

SET(MY_LIBRARY_EXPORT_DIRECTIVE "Q_SLICERQT_EXPORT")
SET(MY_EXPORT_HEADER_PREFIX ${lib_name})
SET(MY_LIBNAME ${lib_name})

CONFIGURE_FILE(
  ${Slicer_SOURCE_DIR}/qSlicerExport.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h
  )
SET(dynamicHeaders
  "${dynamicHeaders};${CMAKE_CURRENT_BINARY_DIR}/${MY_EXPORT_HEADER_PREFIX}Export.h")
    
# --------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------

IF(NOT APPLE)
  SET(SlicerQT_INSTALL_DESTINATION_ARGS RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR})
ELSE()
  SET(SlicerQT_INSTALL_DESTINATION_ARGS BUNDLE DESTINATION ".")
ENDIF()

INSTALL(TARGETS SlicerQT${SlicerQT_EXE_SUFFIX}
  ${SlicerQT_INSTALL_DESTINATION_ARGS}
  COMPONENT Runtime)

# --------------------------------------------------------------------------
# Configure Slicer Launcher (Linux & Windows Only)
# --------------------------------------------------------------------------
IF(Slicer_USE_CTKAPPLAUNCHER)
  INCLUDE(${CTKAPPLAUNCHER_DIR}/CMake/ctkAppLauncher.cmake)

  # Define list of extra 'application to launch' to associate with the launched
  SET(extraApplicationToLaunchListForBuildTree)
  IF(EXISTS "${QT_DESIGNER_EXECUTABLE}")
    ctkAppLauncherAppendExtraAppToLaunchToList(
      LONG_ARG designer
      HELP "Start Qt designer using Slicer plugins"
      PATH ${QT_DESIGNER_EXECUTABLE}
      OUTPUTVAR extraApplicationToLaunchListForBuildTree
      )
  ENDIF()
  SET(executables)
  IF(UNIX)
    LIST(APPEND executables gnome-terminal xterm ddd gdb)
  ELSEIF(WIN32)
    LIST(APPEND executables VisualStudio)
    SET(VisualStudio_EXECUTABLE ${CMAKE_BUILD_TOOL})
  ENDIF()
  FOREACH(executable ${executables})
    FIND_PROGRAM(${executable}_EXECUTABLE ${executable})
    IF(${executable}_EXECUTABLE)
      MESSAGE(STATUS "Enabling Slicer launcher option: --${executable}")
      ctkAppLauncherAppendExtraAppToLaunchToList(
        LONG_ARG ${executable}
        HELP "Start ${executable}"
        PATH ${${executable}_EXECUTABLE}
        OUTPUTVAR extraApplicationToLaunchListForBuildTree
        )
    ENDIF()
  ENDFOREACH()

  INCLUDE(SlicerBlockCTKAppLauncherSettings)
  ctkAppLauncherConfigure(
    TARGET SlicerQT${SlicerQT_EXE_SUFFIX}
    APPLICATION_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
    APPLICATION_NAME Slicer
    SPLASH_IMAGE_PATH ${Slicer_SOURCE_DIR}/Applications/SlicerQT/Resources/Images/SlicerSplashScreen.png
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
  IF(NOT APPLE)
    INSTALL(PROGRAMS "${Slicer_BINARY_DIR}/Slicer${CMAKE_EXECUTABLE_SUFFIX}" DESTINATION ".")

    INSTALL(
      FILES "${Slicer_SOURCE_DIR}/Applications/SlicerQT/Resources/Images/SlicerSplashScreen.png"
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      )
    INSTALL(
      FILES "${Slicer_BINARY_DIR}/SlicerLauncherSettingsToInstall.ini"
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      RENAME SlicerLauncherSettings.ini
      )
  ENDIF()
ENDIF()

# --------------------------------------------------------------------------
# Command line parsing tests
#
# add_test(SlicerCLTest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --help)
# add_test(SlicerCLTest2 ${Slicer_LAUNCHER_EXECUTABLE} --xml --test-mode)
# add_test(SlicerCLTest3 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode -f ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
# add_test(SlicerCLTest4 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --eval "puts testing ,. exit 0")
# add_test(SlicerCLTest5 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --exec "puts testing ,. exit 0")
# add_test(SlicerCLTest6 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
# add_test(SlicerCLTest7 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
# add_test(SlicerScrollTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

# --------------------------------------------------------------------------
# MRML Tests
#
# add_test(SlicerMRMLUndo ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testUndo.tcl)
# add_test(SlicerMRMLVolume ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testVolume.tcl)

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF()
