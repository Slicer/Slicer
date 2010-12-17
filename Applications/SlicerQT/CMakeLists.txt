PROJECT(SlicerQT)

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
#
# get_filename_component(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
# get_filename_component(TCL_DIR "${TCL_LIBRARY_PATH}" PATH)
# get_filename_component(INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
# get_filename_component(INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)

SET(SlicerQT_EXE_SUFFIX)
SET(SlicerQT_EXE_INSTALL ${Slicer_INSTALL_BIN_DIR})
IF(BUILD_SHARED_LIBS)
  SET(SlicerQT_NEED_SHARED_FORWARD 1)
  SET(SlicerQT_EXE_SUFFIX -real)
  SET(SlicerQT_EXE_INSTALL ".")
  SET(SlicerQT_FORWARD_DIR_BUILD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  SET(SlicerQT_FORWARD_DIR_INSTALL "${SlicerQT_EXE_INSTALL}")
  if(WIN32)
    #Nota: Removed the following
    #\"${KWWidgets_RUNTIME_DIRS}\" CONFIG_DIR_POST,
    #\"${TCL_DIR}/bin\",
    #\"${INCR_TCL_LIBRARY_PATH}\",
    #\"${INCR_TK_LIBRARY_PATH}\"
    SET(SlicerQT_FORWARD_PATH_BUILD
      "\"${SlicerQT_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,    \\
       \"${VTK_DIR}/bin\" CONFIG_DIR_POST,                   \\
       \"${ITK_DIR}/bin\" CONFIG_DIR_POST,                   \\
       \"${Teem_DIR}/bin\" CONFIG_DIR_POST")

    #SET(SlicerQT_FORWARD_PATH_INSTALL
    #  "\"../lib/TclTk/bin\",
    #   \"../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}\",
    #   \"../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}\"")

    IF(Slicer_BUILD_QTLOADABLEMODULES)
      SET(SlicerQT_FORWARD_PATH_BUILD
        "${SlicerQT_FORWARD_PATH_BUILD},          \\
         \"${Slicer_HOME}/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}\" CONFIG_DIR_POST")
      SET(SlicerQT_FORWARD_PATH_INSTALL
        "\"../${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}\"")

      # Since ModuleLogic are in Slicer_INSTALL_MODULES_BIN_DIR, let's add it
      SET(Slicer_FORWARD_PATH_BUILD
        "${Slicer_FORWARD_PATH_BUILD},            \\
        \"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\" CONFIG_DIR_POST")
    
    ENDIF(Slicer_BUILD_QTLOADABLEMODULES)
     
    #IF(Slicer_BUILD_MODULES)
    #  SET(Slicer_FORWARD_PATH_BUILD
    #    "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\" CONFIG_DIR_POST")
    #ENDIF(Slicer_BUILD_MODULES)
    
    IF(Slicer_BUILD_CLI)
      SET(SlicerQT_FORWARD_PATH_BUILD
        "${SlicerQT_FORWARD_PATH_BUILD},            \\
        \"${Slicer_HOME}/${Slicer_INSTALL_PLUGINS_BIN_DIR}\" CONFIG_DIR_POST")
    ENDIF(Slicer_BUILD_CLI)
    
  ELSE(WIN32)
  
    #Nota: Removed the following
    #\"${KWWidgets_RUNTIME_DIRS}\",
    #\"${TCL_DIR}/bin\",
    #\"${INCR_TCL_LIBRARY_PATH}\",
    #\"${INCR_TK_LIBRARY_PATH}\",
    SET(SlicerQT_FORWARD_PATH_BUILD
      "\"${Slicer_FORWARD_DIR_BUILD}\",  \\
       \"${VTK_DIR}/bin\",                \\
       \"${ITK_DIR}/bin\",                \\
       \"${Teem_DIR}/bin\",               \\
       \"${OPENGL_gl_LIBRARY_PATH}\"")

    # Nota: Removed the following
    #\"../lib/KWWidgets\",\"../lib/InsightToolkit\",
    #\"../lib/TclTk/lib\",
    #\"../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}\",
    #\"../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}\",
    SET(SlicerQT_FORWARD_PATH_INSTALL
      "\"../${Slicer_INSTALL_LIB_DIR}\",                           \\
       \"../lib/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}\",    \\
       \"../lib/Teem-${Teem_VERSION_MAJOR}.${Teem_VERSION_MINOR}\", \\
       \"../lib/FreeSurfer\",                                       \\
       \"../lib/IGT\",                                              \\
       \"../lib/ITKCommandIO\",                                     \\
       \"../lib/LoadableModule\",                                   \\
       \"../lib/MGHImageIO\",                                       \\
       \"../lib/MRML\",                                             \\
       \"../lib/MRMLCLI\",                                          \\
       \"../lib/MRMLLogic\",                                        \\
       \"../lib/MRMLDisplayableManager\",                           \\
       \"../lib/ModuleDescriptionParser\",                          \\
       \"../lib/MRMLIDImageIO\",                                    \\
       \"../lib/Qdec\",                                             \\
       \"../lib/RemoteIO\",                                         \\
       \"../lib/SlicerIO\",                                         \\
       \"../lib/tclap\",                                            \\
       \"../lib/vtkITK\",                                           \\
       \"../lib/vtkTeem\"")

    IF(Slicer_BUILD_QTLOADABLEMODULES)
      SET(SlicerQT_FORWARD_PATH_BUILD
        "${SlicerQT_FORWARD_PATH_BUILD},                  \\
         \"${Slicer_HOME}/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}\"")
      SET(SlicerQT_FORWARD_PATH_INSTALL
        "${SlicerQT_FORWARD_PATH_INSTALL},         \\
        \"../${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}\"")

      # Since ModuleLogic are in Slicer_INSTALL_MODULES_BIN_DIR, let's add it
      SET(Slicer_FORWARD_PATH_BUILD
        "${Slicer_FORWARD_PATH_BUILD},             \\
        \"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\"")
        
    ENDIF(Slicer_BUILD_QTLOADABLEMODULES)
    
    #IF(Slicer_BUILD_MODULES)
    #  SET(Slicer_FORWARD_PATH_BUILD
    #    "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\"")
    #ENDIF(Slicer_BUILD_MODULES)
    
    IF(Slicer_BUILD_CLI)
      SET(SlicerQT_FORWARD_PATH_BUILD
        "${SlicerQT_FORWARD_PATH_BUILD},         \\
         \"${Slicer_HOME}/${Slicer_INSTALL_PLUGINS_BIN_DIR}\"")
    ENDIF(Slicer_BUILD_CLI)
  ENDIF(WIN32)

#   IF(Slicer_BUILD_MODULES)
#     SET(Slicer_FORWARD_PATH_INSTALL
#       "${Slicer_FORWARD_PATH_INSTALL},\"../${Slicer_INSTALL_MODULES_BIN_DIR}\"")
#   ENDIF(Slicer_BUILD_MODULES)
  
  IF(Slicer_BUILD_CLI)
    SET(SlicerQT_FORWARD_PATH_INSTALL
      "${SlicerQT_FORWARD_PATH_INSTALL},\"../${Slicer_INSTALL_PLUGINS_BIN_DIR}\"")
  ENDIF(Slicer_BUILD_CLI)
  
ENDIF(BUILD_SHARED_LIBS)

# --------------------------------------------------------------------------

#MESSAGE(STATUS SlicerQT_FORWARD_DIR_BUILD:${SlicerQT_FORWARD_DIR_BUILD})
#MESSAGE(STATUS SlicerQT_FORWARD_PATH_BUILD:${SlicerQT_FORWARD_PATH_BUILD})
#MESSAGE(STATUS SlicerQT_FORWARD_PATH_INSTALL:${SlicerQT_FORWARD_PATH_INSTALL})
#MESSAGE(STATUS SlicerQT_FORWARD_DIR_BUILD:${SlicerQT_FORWARD_DIR_BUILD})
#MESSAGE(STATUS SlicerQT_FORWARD_DIR_INSTALL:${SlicerQT_FORWARD_DIR_INSTALL})

# Add main server executables.
SET(SlicerQT_EXE_LIST SlicerQT)

# Add shared link forwarding executables if necessary.
IF(SlicerQT_NEED_SHARED_FORWARD)
  FOREACH(slicer_exe ${SlicerQT_EXE_LIST})
    SET(SlicerQT_FORWARD_EXE ${slicer_exe}${SlicerQT_EXE_SUFFIX})
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/SlicerQT-forward.c.in
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c
      @ONLY)
    ADD_EXECUTABLE(${slicer_exe} WIN32
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c)
    ADD_DEPENDENCIES(${slicer_exe} ${SlicerQT_FORWARD_EXE})
    SET_TARGET_PROPERTIES(${slicer_exe} PROPERTIES LABELS SlicerQT)
    INSTALL(TARGETS ${slicer_exe}
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      COMPONENT Runtime)
  ENDFOREACH(slicer_exe)
ENDIF(SlicerQT_NEED_SHARED_FORWARD)

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
