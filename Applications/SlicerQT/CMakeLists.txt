PROJECT(SlicerQT)

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
#
# get_filename_component(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
# get_filename_component(TCL_DIR "${TCL_LIBRARY_PATH}" PATH)
# get_filename_component(INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
# get_filename_component(INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)

# Some people use Mesa instead of the default system OpenGL.
# Try to incorporate that in the launcher, but NOT for installation.

# if(NOT WIN32)
#   foreach(dependency ${vtkftgl_LIB_DEPENDS})
#     get_filename_component(dependency_lib "${dependency}" NAME_WE)
#     if(dependency_lib STREQUAL "libGL")
#       get_filename_component(OPENGL_gl_LIBRARY_PATH "${dependency}" PATH)
#     endif(dependency_lib STREQUAL "libGL")
#   endforeach(dependency)
# endif(NOT WIN32)

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
    INSTALL(TARGETS ${slicer_exe}
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      COMPONENT Runtime)
  ENDFOREACH(slicer_exe)
ENDIF(SlicerQT_NEED_SHARED_FORWARD)

# --------------------------------------------------------------------------
# Sources
#
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
# Find SLICERLIBCURL (slicercurl)
#
# find_package(SLICERLIBCURL REQUIRED)
# include(${SLICERLIBCURL_USE_FILE})

# --------------------------------------------------------------------------
# Include dirs
#
set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  #${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  #${NAVITRACK_INC}
  #${IGSTK_INCLUDE_DIRS}
#   ${Volumes_SOURCE_DIR}
#   ${Volumes_BINARY_DIR}
#   ${Volumes_Init_SOURCE_DIR}
#   ${Volumes_Init_BINARY_DIR}
#   ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
#   ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
#   ${GradientAnisotropicDiffusionFilter_Init_SOURCE_DIR}
#   ${GradientAnisotropicDiffusionFilter_Init_BINARY_DIR}
#   ${SlicerTractographyDisplay_SOURCE_DIR}
#   ${SlicerTractographyDisplay_BINARY_DIR}
#   ${SlicerTractographyDisplay_Init_SOURCE_DIR}
#   ${SlicerTractographyDisplay_Init_BINARY_DIR}
#   ${SlicerTractographyFiducialSeeding_SOURCE_DIR}
#   ${SlicerTractographyFiducialSeeding_BINARY_DIR}
#   ${SlicerTractographyFiducialSeeding_Init_SOURCE_DIR}
#   ${SlicerTractographyFiducialSeeding_Init_BINARY_DIR}
#   ${EMSegment_SOURCE_DIR}
#   ${EMSegment_BINARY_DIR}
#   ${EMSegment_Init_SOURCE_DIR}
#   ${EMSegment_Init_BINARY_DIR}
#   ${RealTimeImaging_SOURCE_DIR}
#   ${RealTimeImaging_BINARY_DIR}
#   ${RealTimeImaging_Init_SOURCE_DIR}
#   ${RealTimeImaging_Init_BINARY_DIR}
#   ${MRAblation_SOURCE_DIR}
#   ${MRAblation_BINARY_DIR}
#   ${MRAblation_Init_SOURCE_DIR}
#   ${MRAblation_Init_BINARY_DIR}
#   ${LabelStatistics_SOURCE_DIR}       
#   ${LabelStatistics_BINARY_DIR}
#   ${LabelStatistics_Init_SOURCE_DIR}
#   ${LabelStatistics_Init_BINARY_DIR}
#   ${WFEngineModule_SOURCE_DIR}
#   ${WFEngineModule_BINARY_DIR}
#   ${WFEngineModule_Init_SOURCE_DIR}
#   ${WFEngineModule_Init_BINARY_DIR}
#   ${CommandLineModule_SOURCE_DIR}
#   ${CommandLineModule_BINARY_DIR}
#   ${CommandLineModule_Init_SOURCE_DIR}
#   ${CommandLineModule_Init_BINARY_DIR}
#   ${ScriptedModule_SOURCE_DIR}
#   ${ScriptedModule_BINARY_DIR}
#   ${SlicerDaemon_SOURCE_DIR}
#   ${SlicerDaemon_BINARY_DIR}
#   ${SlicerDaemon_Init_SOURCE_DIR}
#   ${SlicerDaemon_Init_BINARY_DIR}
  )

# if(Slicer_BUILD_MODULES)
#   add_definitions(-DSlicer_BUILD_MODULES)
# endif(Slicer_BUILD_MODULES)
# 
# if(Slicer_BUILD_CLI)
#   add_definitions(-DSlicer_BUILD_CLI)
# endif(Slicer_BUILD_CLI)

# if(Slicer_USE_PYTHON AND PYTHON_INCLUDE_PATH)
#   set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH})
#   if(WIN32)
#     set(include_dirs ${include_dirs} ${PYTHON_INCLUDE_PATH}/../PC)
#   endif(WIN32)
# endif(Slicer_USE_PYTHON AND PYTHON_INCLUDE_PATH)

INCLUDE_DIRECTORIES(${include_dirs})

# --------------------------------------------------------------------------
# with the WIN32_CONSOLE you can see diagnostic output on in a DOS-style
# cmd window
#
# option(Slicer_BUILD_WIN32_CONSOLE 
#   "Build Slicer3 executable as a console app on windows (allows debug output)" 
#   OFF
#   )
# 
# if(Slicer_BUILD_WIN32_CONSOLE)
#   add_executable(Slicer3${Slicer_EXE_SUFFIX} 
#     ${SRC} 
#     ${RC_FILENAME}
#     )
# else(Slicer_BUILD_WIN32_CONSOLE)
#   add_executable(
#     Slicer3${Slicer_EXE_SUFFIX} WIN32 
#     ${SRC} 
#     ${RC_FILENAME}
#     )
# endif(Slicer_BUILD_WIN32_CONSOLE)

# --------------------------------------------------------------------------
# Options
#
# if(WFENGINEMODULE)
#   set(WFENGINE_LINK_LIB ${WFENGINE_LINK_LIB} WFEngineModule)
# else(WFENGINEMODULE)
#   add_definitions(-DWFENGINE_DEBUG)
# endif(WFENGINEMODULE)

# if(LABELSTATISTICS)
#   set(LABELSTATISTICS_LINK_LIB LabelStatistics)
# else(LABELSTATISTICS)
#   add_definitions(-DLABELSTATISTICS_DEBUG)
# endif(LABELSTATISTICS)

# --------------------------------------------------------------------------
# Link
# Important: remove RemoteIO when using CMake 2.6. Couldn't be part of 
# Slicer_LIBRARIES right now because of the Curl lib RemoteIO is using is
# STATIC, but that's fixed in 2.6.
#
#target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
#  ${Slicer_Libs_LIBRARIES}
#  ${Slicer_Base_LIBRARIES}
#   RemoteIO
#   ${KWWidgets_LIBRARIES}
#  ${SLICERLIBCURL_LIBRARIES}
#  )

# if(Slicer_BUILD_MODULES)
#   target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
#     Volumes
#     CommandLineModule
#     ScriptedModule
#     SlicerDaemon
#     SlicerTractographyDisplay
#     SlicerTractographyFiducialSeeding
#     )
# else(Slicer_BUILD_MODULES)
#   # if Slicer_BUILD_CLI is ON, we still need at least the CommandLineModule
#   # so that we can have CLI, without all the other modules
#   if(Slicer_BUILD_CLI)
#     target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
#       CommandLineModule
#       )
#   endif(Slicer_BUILD_CLI)
# endif(Slicer_BUILD_MODULES)

# if(Slicer_USE_PYTHON AND PYTHON_LIBRARIES)
#   target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} SlicerBaseGUIPython)
#   if(WIN32 AND PYTHON_DEBUG_LIBRARIES)
#     target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
#       debug ${PYTHON_DEBUG_LIBRARIES}
#       optimized ${PYTHON_LIBRARIES})
#   else(WIN32 AND PYTHON_DEBUG_LIBRARIES)
#     target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} ${PYTHON_LIBRARIES})
#   endif(WIN32 AND PYTHON_DEBUG_LIBRARIES)
#   if(NOT WIN32)
#     # Linux/Unix requires util library, but mac doesn't
#     if(APPLE OR UNIX)
#       target_link_libraries(Slicer3${Slicer_EXE_SUFFIX})
#     else(APPLE OR UNIX)
#       target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} util)
#     endif(APPLE OR UNIX)
#   endif(NOT WIN32)
# endif(Slicer_USE_PYTHON AND PYTHON_LIBRARIES)
    
# --------------------------------------------------------------------------
# Build the library

SET(lib_name "qSlicerQT")

#slicer3_get_persistent_property(Slicer_Base_LIBRARIES tmp)
#slicer3_set_persistent_property(Slicer_Base_LIBRARIES ${tmp} ${lib_name})

ADD_LIBRARY(${lib_name}
  ${SlicerQT_SRCS}
  ${SlicerQT_UI_CXX}
  ${SlicerQT_QRC_SRCS}
)

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

ADD_EXECUTABLE(SlicerQT${SlicerQT_EXE_SUFFIX}
  Main.cxx)
  
TARGET_LINK_LIBRARIES(SlicerQT${SlicerQT_EXE_SUFFIX}
  ${lib_name}
  )

#-----------------------------------------------------------------------------
# Configure
#
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
#
INSTALL(TARGETS SlicerQT${SlicerQT_EXE_SUFFIX}
  DESTINATION ${Slicer_INSTALL_BIN_DIR}
  COMPONENT Runtime)

# Install headers
#FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
#INSTALL(FILES
#  ${headers}
#  ${dynamicHeaders}
#  DESTINATION ${Slicer_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
#  )
    
# --------------------------------------------------------------------------
# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

# include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
# kwwidgets_copy_tcl_tk_support_files("${Slicer_BINARY_DIR}/lib")
# kwwidgets_install_tcl_tk_support_files("/lib")

# --------------------------------------------------------------------------
# Use the command line utilities
#
# find_package(GenerateCLP REQUIRED)
# if(GenerateCLP_FOUND)
#   include(${GenerateCLP_USE_FILE})
# endif(GenerateCLP_FOUND)
# 
# generateclp(${SRC} Slicer3.xml)

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
