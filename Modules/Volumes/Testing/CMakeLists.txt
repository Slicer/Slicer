SET(KIT VolumesModule)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  DiffusionTensorEditorTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests MRML vtkCommon)

SET( VolumesModule_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${VolumesModule_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


add_test(DiffusionEditorTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorTest.tcl)
add_test(DiffusionEditorGradients ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditor_GradientsTest.tcl)
add_test(DiffusionEditorGlyphsTracts ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorGlyphsTractsTest.tcl)
add_test(DiffusionEditorTensor ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorTensorTest.tcl)
set(SHORT_TEST_TIMEOUT 90)  ## Sometimes when the GUI crashes, it won't exit properly, and then the tests have to wait until they reach 1500 seconds before they time out
set_tests_properties( DiffusionEditorTensor PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT} )

add_test( DiffusionTensorEditorTest1 
  ${VolumesModule_TESTS} DiffusionTensorEditorTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/Testing/DiffusionEditorTensorTest.tcl
  )
