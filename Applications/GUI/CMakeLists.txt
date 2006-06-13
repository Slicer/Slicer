PROJECT( GUI )

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
  ${SlicerDaemon_SOURCE_DIR}
  ${SlicerDaemon_BINARY_DIR}
  ${SlicerDaemon_Init_SOURCE_DIR}
  ${SlicerDaemon_Init_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )

#ADD_EXECUTABLE (Slicer3 WIN32 ${SRC})
ADD_EXECUTABLE (Slicer3 ${SRC})
TARGET_LINK_LIBRARIES( Slicer3 
    SlicerBaseLogic
    SlicerBaseGUI
    MRML
    GradientAnisotropicDiffusionFilter
    SlicerDaemon
    ModuleDescriptionParser
    ModulesBaseClasses
    vtkITK
    ITKCommon 
    ITKIO
    ${KWWidgets_LIBRARIES} )
INSTALL_TARGETS(/bin Slicer3)

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
  "${CMAKE_CURRENT_BINARY_DIR}" "Slicer3Launcher" "" "Slicer3")


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
ADD_TEST(Slicer3CLTest5 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --exec "puts testing ,. exit 0")
ADD_TEST(Slicer3CLTest6 ${EXECUTABLE_OUTPUT_PATH}/Slicer3 --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)

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
