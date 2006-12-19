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

INCLUDE_DIRECTORIES(
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_BINARY_DIR}
#  ${EMSegment_SOURCE_DIR}
#  ${EMSegment_BINARY_DIR}
#  ${EMSegment_Init_SOURCE_DIR}
#  ${EMSegment_Init_BINARY_DIR}
  ${QueryAtlas_SOURCE_DIR}
  ${QueryAtlas_BINARY_DIR}
  ${QueryAtlas_Init_SOURCE_DIR}
  ${QueryAtlas_Init_BINARY_DIR}
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
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  ${OPENTRACKER_INC_DIR}
  )

#
# with the WIN32_CONSOLE you can see diagnostic output on in a DOS-style
# cmd window
#
OPTION(Slicer3_BUILD_WIN32_CONSOLE "Build Slicer3 executable as a console app on windows (allows debug output)" OFF)
IF ( Slicer3_BUILD_WIN32_CONSOLE )
  ADD_EXECUTABLE (Slicer3${SLICER_EXE_SUFFIX} ${SRC})
ELSE ( Slicer3_BUILD_WIN32_CONSOLE )
  ADD_EXECUTABLE (Slicer3${SLICER_EXE_SUFFIX} WIN32 ${SRC})
ENDIF ( Slicer3_BUILD_WIN32_CONSOLE )

TARGET_LINK_LIBRARIES( Slicer3${SLICER_EXE_SUFFIX}
    SlicerBaseLogic
    SlicerBaseGUI
    MRML
    GradientAnisotropicDiffusionFilter
#    EMSegment
    QueryAtlas
    CommandLineModule
    ScriptedModule
    SlicerDaemon
    ModuleDescriptionParser
    ModulesBaseClasses
    vtkITK
    ITKCommon 
    ITKIO
    ${KWWidgets_LIBRARIES} )
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

IF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)
  IF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
    INCLUDE(${VTK_TCL_TK_MACROS_MODULE})
    VTK_COPY_TCL_TK_SUPPORT_FILES_TO_DIR(
      ${VTK_TCL_SUPPORT_LIBRARY_PATH} ${VTK_TK_SUPPORT_LIBRARY_PATH}
      "${Slicer3_BINARY_DIR}/lib")
    VTK_COPY_TCL_TK_SUPPORT_FILES_TO_DIR(
      ${VTK_TCL_SUPPORT_LIBRARY_PATH} ${VTK_TK_SUPPORT_LIBRARY_PATH}
      "/lib" INSTALL)
  ENDIF(VTK_TCL_SUPPORT_LIBRARY_PATH AND VTK_TK_SUPPORT_LIBRARY_PATH)
ENDIF(VTK_TCL_TK_COPY_SUPPORT_LIBRARY)

#
# use the command line utilities
#
INCLUDE(${Slicer3_SOURCE_DIR}/Libs/GenerateCLP/UseGenerateCLP.cmake)

GENERATECLP(${SRC} Slicer3.xml)

#
# Command line parsing tests
#

ADD_TEST(Slicer3CLTest1 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --help)
ADD_TEST(Slicer3CLTest2 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --xml)
ADD_TEST(Slicer3CLTest3 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 -f ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
ADD_TEST(Slicer3CLTest4 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --eval "puts testing ,. exit 0")
ADD_TEST(Slicer3CLTest5 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --exec "puts testing ,. $::slicer3::Application Exit")
ADD_TEST(Slicer3CLTest6 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
ADD_TEST(Slicer3CLTest7 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)

#
# MRML Tests
#

ADD_TEST(Slicer3MRMLUndo ${EXECUTABLE_OUTPUT_PATH}/Slicer3 -f ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/testUndo.tcl)
ADD_TEST(Slicer3MRMLVolume ${EXECUTABLE_OUTPUT_PATH}/Slicer3 -f ${Slicer3_SOURCE_DIR}/Libs/MRML/Testing/testVolume.tcl)

#
# make the testing
#
IF (Slicer3_BUILD_TESTING)
  SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
