SET(KIT VolumesModule)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  DiffusionTensorEditorTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests SlicerBaseGUI)

SET( VolumesModule_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(VolumesModule_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${VolumesModule_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


add_test(DiffusionEditorTest ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTest.tcl)
add_test(DiffusionEditorGradients ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditor_GradientsTest.tcl)
add_test(DiffusionEditorGlyphsTracts ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorGlyphsTractsTest.tcl)
add_test(DiffusionEditorTensor ${Slicer_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl)
set(SHORT_TEST_TIMEOUT 300)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out
set_tests_properties( DiffusionEditorTensor PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )

add_test( DiffusionTensorEditorTest1 
  ${LAUNCH_EXE} ${VolumesModule_TESTS} DiffusionTensorEditorTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl
  )
