set(KIT BaseGUI)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSlicerModelsGUITest1.cxx
  vtkSlicerModuleGUITest1.cxx
  vtkSlicerComponentGUITest1.cxx
  vtkSlicerViewerTest.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

set(LIBRARY_NAME ${PROJECT_NAME})

# --------------------------------------------------------------------------
# Testing

set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out

add_test(Slicer3GUITest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
add_test(Slicer3GUITest2 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestProgressDialog.tcl)
add_test(Slicer3FiducialsTest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestFiducials.tcl)
add_test(Slicer3FiducialsTest2 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestFiducialsAdd.tcl)
add_test(Slicer3FiducialsTestGlyphs ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestFiducialsGlyphs.tcl)
add_test(Slicer3FiducialsTestChangeGlyphs ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestFiducialsAddSymbolScale.tcl)
add_test(Slicer3RestoreSceneSnapshotCompareViewTest1 ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --script ${Slicer_SOURCE_DIR}/Base/GUI/Testing/TestRestoreSceneSnapshotCompareView.tcl)

set_tests_properties( Slicer3FiducialsTest1 PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )
set_tests_properties( Slicer3FiducialsTest2 PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )
set_tests_properties( Slicer3RestoreSceneSnapshotCompareViewTest1 PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests
  ${lib_name}
  SlicerBaseGUI
  )

macro( SIMPLE_TEST  TESTNAME )
  add_test( ${TESTNAME} ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SlicerQT
    )
endmacro()

SIMPLE_TEST( vtkSlicerModelsGUITest1 )
SIMPLE_TEST( vtkSlicerModuleGUITest1 )
SIMPLE_TEST( vtkSlicerComponentGUITest1 )
SIMPLE_TEST( vtkSlicerViewerTest )
