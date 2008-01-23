PROJECT( GUI )

#########################################################################
# Enable shared link forwarding support if needed.
SET(SLICER_EXE_SUFFIX)
SET(SLICER_EXE_INSTALL ${SLICER_INSTALL_BIN_DIR})
IF(BUILD_SHARED_LIBS)
    SET(SLICER_NEED_SHARED_FORWARD 1)
    SET(SLICER_EXE_SUFFIX -real)
    SET(SLICER_EXE_INSTALL ".")
    SET(SLICER_FORWARD_DIR_BUILD "${EXECUTABLE_OUTPUT_PATH}")
    SET(SLICER_FORWARD_DIR_INSTALL "${SLICER_EXE_INSTALL}")
    SET(SLICER_FORWARD_PATH_BUILD "\"${SLICER_FORWARD_DIR_BUILD}\",\"${KWWidgets_RUNTIME_DIRS}\",\"${VTK_DIR}/bin\"")
  IF(WIN32)
    SET(SLICER_FORWARD_PATH_INSTALL
      "\"../lib/Slicer3/tcl/bin\"")
  ELSE(WIN32)
    SET(SLICER_FORWARD_PATH_INSTALL
      "\"../lib\",\"../lib/KWWidgets\",\"../lib/InsightToolkit\",\"../lib/Slicer3/tcl/lib\"")
  ENDIF(WIN32)
ENDIF(BUILD_SHARED_LIBS)
# Add main server executables.
SET(SLICER_EXE_LIST Slicer3)

# Add shared link forwarding executables if necessary.
IF(SLICER_NEED_SHARED_FORWARD)
  FOREACH(slicer_exe ${SLICER_EXE_LIST})
    SET(SLICER_FORWARD_EXE ${slicer_exe}${SLICER_EXE_SUFFIX})
    CONFIGURE_FILE(
      ${CMAKE_CURRENT_SOURCE_DIR}/Slicer3-forward.c.in
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c
      @ONLY IMMEDIATE)
    ADD_EXECUTABLE(${slicer_exe} WIN32
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c)
    ADD_DEPENDENCIES(${slicer_exe} ${slicer_exe}${SLICER_EXE_SUFFIX})
    INSTALL(TARGETS ${slicer_exe}
      DESTINATION bin
      COMPONENT Runtime)
  ENDFOREACH(slicer_exe)
ENDIF(SLICER_NEED_SHARED_FORWARD)

SET (SRC
    Slicer3.cxx
)

# --------------------------------------------------------------------------
# Include dirs
# MESSAGE(STATUS "Setting up dir_list, Qdec bin dir = ${Qdec_BINARY_DIR}, Qdec src dir = ${Qdec_SOURCE_DIR}")

SET(dir_list
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${IGT_SOURCE_DIR}
  ${IGT_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_BINARY_DIR}
  ${SlicerTractographyDisplay_SOURCE_DIR}
  ${SlicerTractographyDisplay_BINARY_DIR}
  ${SlicerTractographyDisplay_Init_SOURCE_DIR}
  ${SlicerTractographyDisplay_Init_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_Init_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_Init_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  ${EMSegment_Init_SOURCE_DIR}
  ${EMSegment_Init_BINARY_DIR}
  ${RealTimeImaging_SOURCE_DIR}
  ${RealTimeImaging_BINARY_DIR}
  ${RealTimeImaging_Init_SOURCE_DIR}
  ${RealTimeImaging_Init_BINARY_DIR}
  ${MRAblation_SOURCE_DIR}
  ${MRAblation_BINARY_DIR}
  ${MRAblation_Init_SOURCE_DIR}
  ${MRAblation_Init_BINARY_DIR}
  ${NeuroNav_SOURCE_DIR}
  ${NeuroNav_BINARY_DIR}
  ${NeuroNav_Init_SOURCE_DIR}
  ${NeuroNav_Init_BINARY_DIR}
  ${Qdec_SOURCE_DIR}
  ${Qdec_BINARY_DIR}
  ${QdecModule_SOURCE_DIR}
  ${QdecModule_BINARY_DIR}
  ${QdecModule_Init_SOURCE_DIR}
  ${QdecModule_Init_BINARY_DIR}
  ${QueryAtlas_SOURCE_DIR}
  ${QueryAtlas_BINARY_DIR}
  ${QueryAtlas_Init_SOURCE_DIR}
  ${QueryAtlas_Init_BINARY_DIR}
  ${VolumeRenderingModule_SOURCE_DIR}
  ${VolumeRenderingModule_BINARY_DIR}
  ${VolumeRenderingModule_Init_SOURCE_DIR}
  ${VolumeRenderingModule_Init_BINARY_DIR}
  ${LabelStatistics_SOURCE_DIR}       
  ${LabelStatistics_BINARY_DIR}
  ${LabelStatistics_Init_SOURCE_DIR}
  ${LabelStatistics_Init_BINARY_DIR}
  ${WFEngineModule_SOURCE_DIR}
  ${WFEngineModule_BINARY_DIR}
  ${WFEngineModule_Init_SOURCE_DIR}
  ${WFEngineModule_Init_BINARY_DIR}
  ${CommandLineModule_SOURCE_DIR}
  ${CommandLineModule_BINARY_DIR}
  ${CommandLineModule_Init_SOURCE_DIR}
  ${CommandLineModule_Init_BINARY_DIR}
  ${ScriptedModule_SOURCE_DIR}
  ${ScriptedModule_BINARY_DIR}
  ${SlicerDaemon_SOURCE_DIR}
  ${SlicerDaemon_BINARY_DIR}
  ${SlicerDaemon_Init_SOURCE_DIR}
  ${SlicerDaemon_Init_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${VTK_INCLUDE_DIR}
  ${NAVITRACK_INC}
  ${IGSTK_INCLUDE_DIRS}
  )

IF(BUILD_MODULES)
  ADD_DEFINITIONS(-DBUILD_MODULES)
ENDIF(BUILD_MODULES)

IF(BUILD_CLI)
  ADD_DEFINITIONS(-DBUILD_CLI)
ENDIF(BUILD_CLI)

IF(USE_PYTHON AND PYTHON_INCLUDE_PATH)
  SET(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
  ADD_DEFINITIONS(-DUSE_PYTHON)
ENDIF(USE_PYTHON AND PYTHON_INCLUDE_PATH)

INCLUDE_DIRECTORIES(${dir_list})

# On Win32 platforms, let's configure the KWWidgets sample resource file
# to get a nice application icon and some additional information.
# This is *mandatory* for a static build otherwise the static Tk resources
# won't be included and images like checkboxes will not appear.

# NOTE: this is commented for now until we get rid of extra include directives.
# We have 90 (!) at this point and rc.exe can *not* handle such number.

#IF(WIN32 AND NOT BORLAND AND NOT CYGWIN)
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
#  SET(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/Slicer3.rc")
#  KWWidgets_CREATE_RC_FILE(
#    RC_FILENAME "${RC_FILENAME}"
#    RC_APPLICATION_NAME "Slicer3"
#    RC_COMPANY_NAME "NA-MIC")
#ENDIF(WIN32 AND NOT BORLAND AND NOT CYGWIN)

#
# with the WIN32_CONSOLE you can see diagnostic output on in a DOS-style
# cmd window
#
OPTION(Slicer3_BUILD_WIN32_CONSOLE "Build Slicer3 executable as a console app on windows (allows debug output)" OFF)
IF ( Slicer3_BUILD_WIN32_CONSOLE )
  ADD_EXECUTABLE (Slicer3${SLICER_EXE_SUFFIX} ${SRC} ${RC_FILENAME})
ELSE ( Slicer3_BUILD_WIN32_CONSOLE )
  ADD_EXECUTABLE (Slicer3${SLICER_EXE_SUFFIX} WIN32 ${SRC} ${RC_FILENAME})
ENDIF ( Slicer3_BUILD_WIN32_CONSOLE )

IF(WFENGINEMODULE)
SET(WFENGINE_LINK_LIB ${WFENGINE_LINK_LIB} WFEngineModule)
ELSE(WFENGINEMODULE)
ADD_DEFINITIONS(-DWFENGINE_DEBUG)
ENDIF(WFENGINEMODULE)

# Option for VolumeRendering
IF(VOLUMERENDERINGMODULE)
 SET(VOLUMERENDERINGMODULE_LINK_LIB VolumeRenderingModule)
ELSE(VOLUMERENDERINGMODULE)
 ADD_DEFINITIONS(-DVOLUMERENDERINGMODULE_DEBUG)
ENDIF(VOLUMERENDERINGMODULE)

# Option for LabelStatistics
IF(LABELSTATISTICS)
 SET(LABELSTATISTICS_LINK_LIB LabelStatistics)
ELSE(LABELSTATISTICS)
 ADD_DEFINITIONS(-DLABELSTATISTICS_DEBUG)
ENDIF(LABELSTATISTICS)

TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX}
    SlicerBaseLogic
    SlicerBaseGUI
    MRML
    IGT
    ModuleDescriptionParser
    vtkITK
    ITKCommon 
    ITKIO
    ${KWWidgets_LIBRARIES}
    Qdec
)

IF(BUILD_MODULES)
  TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX}
    GradientAnisotropicDiffusionFilter
    EMSegment
    #   RealTimeImaging
    #   MRAblation
    ${VOLUMERENDERINGMODULE_LINK_LIB}
    ${LABELSTATISTICS_LINK_LIB} 
    NeuroNav
    QueryAtlas
    CommandLineModule
    ScriptedModule
    SlicerDaemon
    ${WFENGINE_LINK_LIB}
    QdecModule
    SlicerTractographyDisplay
    SlicerTractographyFiducialSeeding
    )
ENDIF(BUILD_MODULES)

IF(USE_PYTHON AND PYTHON_LIBRARIES)
  ADD_DEFINITIONS(-DUSE_PYTHON)
  TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX} SlicerGUIPython)
  IF (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX}
                            debug ${PYTHON_DEBUG_LIBRARIES}
                            optimized ${PYTHON_LIBRARIES})
  ELSE (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX} ${PYTHON_LIBRARIES})
  ENDIF (WIN32 AND PYTHON_DEBUG_LIBRARIES)
  IF(NOT WIN32)
     # Linux/Unix requires util library, but mac doesn't
     IF (APPLE)
       TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX} )
     ELSE (APPLE)
       TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX} util )
     ENDIF (APPLE)
  ENDIF(NOT WIN32)
ENDIF(USE_PYTHON AND PYTHON_LIBRARIES)

INSTALL_TARGETS(/bin Slicer3${SLICER_EXE_SUFFIX})

# Generate a lightweight C launcher for a *specific* executable.
# The launcher sets up all the environments variables (PATH, TCLLIBPATH,
# LD_LIBRARY_PATH, etc.) required by this executable and some known external 
# third-party dependencies like VTK, ITK, SOV, etc., before launching the
# executable itself. Note that the paths to many of the external libraries are
# hard-coded in the launcher: do not copy the launcher to your installation 
# tree, if any (ultimately, all the DLLs needed by a project should/could be
# stored together in the installation tree, and a launcher should not be
# needed in that case).

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
KWWidgets_GENERATE_SETUP_PATHS_LAUNCHER(
  "${CMAKE_CURRENT_BINARY_DIR}" "Slicer3Launcher" "" "Slicer3-real" "${TEEM_DIR}/bin")

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
KWWidgets_COPY_TCL_TK_SUPPORT_FILES("${Slicer3_BINARY_DIR}/lib")
KWWidgets_INSTALL_TCL_TK_SUPPORT_FILES("/lib")

#
# use the command line utilities
#
FIND_PACKAGE(GenerateCLP REQUIRED)
IF(GenerateCLP_FOUND)
  INCLUDE(${GenerateCLP_USE_FILE})
ENDIF(GenerateCLP_FOUND)

GENERATECLP(${SRC} Slicer3.xml)

#
# Command line parsing tests
#

ADD_TEST(Slicer3CLTest1 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --help)
ADD_TEST(Slicer3CLTest2 ${Slicer3_BINARY_DIR}/Slicer3 --xml --no-splash)
ADD_TEST(Slicer3CLTest3 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash -f ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
ADD_TEST(Slicer3CLTest4 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --eval "puts testing ,. exit 0")
ADD_TEST(Slicer3CLTest5 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --exec "puts testing ,. exit 0")
ADD_TEST(Slicer3CLTest6 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
ADD_TEST(Slicer3CLTest7 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
ADD_TEST(Slicer3ScrollTest ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

#
# MRML Tests
#

ADD_TEST(Slicer3MRMLUndo ${Slicer3_BINARY_DIR}/Slicer3 --no-splash -f ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/testUndo.tcl)
ADD_TEST(Slicer3MRMLVolume ${Slicer3_BINARY_DIR}/Slicer3 --no-splash -f ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/testVolume.tcl)

#
# make the testing
#
IF (Slicer3_BUILD_TESTING)
  SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
