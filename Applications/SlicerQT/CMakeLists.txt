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

# HACK Since we don't depend on SlicerBaseGUI, remove it from the list
SET(Slicer_BaseQT_LIBRARIES ${Slicer_Base_LIBRARIES})
LIST(REMOVE_ITEM Slicer_BaseQT_LIBRARIES SlicerBaseGUI)

SET(SlicerQT_LIBRARIES 
  ${Slicer_BaseQT_LIBRARIES}
  )

IF(Slicer_USE_PYTHONQT)
  SET(SlicerQT_LIBRARIES 
    ${SlicerQT_LIBRARIES}
    qSlicerBaseQTCorePythonQt
    qSlicerBaseQTGUIPythonQt
    )
ENDIF(Slicer_USE_PYTHONQT)

target_link_libraries(${lib_name} 
  ${SlicerQT_LIBRARIES}
  )

# Apply user-defined properties to the library target.
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

SET(SlicerQT_EXE_SUFFIX -real)
ADD_EXECUTABLE(SlicerQT${SlicerQT_EXE_SUFFIX}
  Main.cxx)
SET_TARGET_PROPERTIES(SlicerQT${SlicerQT_EXE_SUFFIX} PROPERTIES LABELS SlicerQT)
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

INSTALL(TARGETS SlicerQT${SlicerQT_EXE_SUFFIX}
  DESTINATION ${Slicer_INSTALL_BIN_DIR}
  COMPONENT Runtime)

# --------------------------------------------------------------------------
# Configure Slicer Launcher
# --------------------------------------------------------------------------

IF(Slicer_USE_CTKAPPLAUNCHER)
  INCLUDE(${CTKAPPLAUNCHER_DIR}/CMake/ctkAppLauncher.cmake)
  INCLUDE(${Slicer_CMAKE_DIR}/SlicerCTKAppLauncherSettings.cmake)
  ctkAppLauncherConfigure(
    TARGET SlicerQT${SlicerQT_EXE_SUFFIX}
    APPLICATION_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
    APPLICATION_NAME Slicer
    SPLASH_IMAGE_PATH ${Slicer_SOURCE_DIR}/Applications/SlicerQT/Resources/Images/SlicerSplashScreen.png
    SPLASH_IMAGE_INSTALL_SUBDIR ${Slicer_INSTALL_BIN_DIR}
    SPLASHSCREEN_HIDE_DELAY_MS 3000
    HELP_SHORT_ARG "-h"
    HELP_LONG_ARG "--help"
    NOSPLASH_LONG_ARG "--no-splash"
    DESTINATION_DIR ${Slicer_BINARY_DIR}
    LIBRARY_PATHS_BUILD "${SLICER_LIBRARY_PATHS_BUILD}"
    PATHS_BUILD "${SLICER_PATHS_BUILD}"
    ENVVARS_BUILD "${SLICER_ENVVARS_BUILD}"
    LIBRARY_PATHS_INSTALLED "${SLICER_LIBRARY_PATHS_INSTALLED}"
    PATHS_INSTALLED "${SLICER_PATHS_INSTALLED}"
    ENVVARS_INSTALLED "${SLICER_ENVVARS_INSTALLED}"
    )
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

# --------------------------------------------------------------------------
# Command line parsing tests
#
# add_test(Slicer3CLTest1 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --help)
# add_test(Slicer3CLTest2 ${Slicer_BINARY_DIR}/Slicer3 --xml --test-mode)
# add_test(Slicer3CLTest3 ${Slicer_BINARY_DIR}/Slicer3 --test-mode -f ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
# add_test(Slicer3CLTest4 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --eval "puts testing ,. exit 0")
# add_test(Slicer3CLTest5 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --exec "puts testing ,. exit 0")
# add_test(Slicer3CLTest6 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
# add_test(Slicer3CLTest7 ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
# add_test(Slicer3ScrollTest ${Slicer_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

# --------------------------------------------------------------------------
# MRML Tests
#
# add_test(Slicer3MRMLUndo ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testUndo.tcl)
# add_test(Slicer3MRMLVolume ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testVolume.tcl)

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
