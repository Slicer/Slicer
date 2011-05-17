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


add_test(NAME DiffusionEditorTest COMMAND ${Slicer_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTest.tcl)

add_test(NAME DiffusionEditorGradients COMMAND ${Slicer_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditor_GradientsTest.tcl)

add_test(NAME DiffusionEditorGlyphsTracts COMMAND ${Slicer_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorGlyphsTractsTest.tcl)

add_test(NAME DiffusionEditorTensor COMMAND ${Slicer_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl)
# Sometimes when the GUI crashes, it won't exit properly, and then 
# the tests have to wait until they reach 1500 seconds before they time out
set(SHORT_TEST_TIMEOUT 300)
set_tests_properties(DiffusionEditorTensor PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT})

add_test(NAME DiffusionTensorEditorTest1 
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> DiffusionTensorEditorTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl
  )
