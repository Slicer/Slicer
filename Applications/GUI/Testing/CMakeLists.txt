project(GUITesting)
#####################

# --------------------------------------------------------------------------
# Command line parsing tests
#
add_test(SlicerCLTest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --help)
add_test(SlicerCLTest2 ${Slicer_LAUNCHER_EXECUTABLE} --xml --test-mode)
add_test(SlicerCLTest3 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode -f ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(SlicerCLTest4 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --eval "puts testing ,. exit 0")
add_test(SlicerCLTest5 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --exec "puts testing ,. exit 0")
add_test(SlicerCLTest6 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(SlicerCLTest7 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
add_test(SlicerCLTest8 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-modules --eval exit)
add_test(SlicerCLTest9 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-modules --exec "exit")
add_test(SlicerScrollTest ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

# --------------------------------------------------------------------------
# MRML Tests
#
add_test(SlicerMRMLUndo ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testUndo.tcl)
add_test(SlicerMRMLVolume ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash -f ${Slicer_HOME}/share/MRML/Testing/testVolume.tcl)


# --------------------------------------------------------------------------
find_package(GenerateCLP REQUIRED)
include(${GenerateCLP_USE_FILE})

find_package(vtkITK REQUIRED)

link_directories(${ModuleDescriptionParser_BINARY_DIR})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(CLP EventBrokerTest)

set(${CLP}_SOURCE ${CLP}.cxx)
generateclp(${CLP}_SOURCE ${CLP}.xml ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
link_directories(${vtkITK_LIB_DIR})
add_executable(${CLP} ${${CLP}_SOURCE})
target_link_libraries(${CLP}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  )

include_directories(
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
add_test(EventBrokerTest 
  ${LAUNCH_EXE} EventBrokerTest --logFileName ${TEMP}/EventLog.dot --graphFile ${TEMP}/EventBroker.dot)
