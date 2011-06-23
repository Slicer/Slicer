set(KIT VolumesModule)
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  DiffusionTensorEditorTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests SlicerBaseGUI)


add_test(NAME DiffusionEditorTest COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTest.tcl)

add_test(NAME DiffusionEditorGradients COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditor_GradientsTest.tcl)

add_test(NAME DiffusionEditorGlyphsTracts COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorGlyphsTractsTest.tcl)

add_test(NAME DiffusionEditorTensor COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --test-mode --no-splash --script ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl)
# Sometimes when the GUI crashes, it won't exit properly, and then
# the tests have to wait until they reach 1500 seconds before they time out
set(SHORT_TEST_TIMEOUT 300)
set_tests_properties(DiffusionEditorTensor PROPERTIES TIMEOUT ${SHORT_TEST_TIMEOUT})

add_test(NAME DiffusionTensorEditorTest1
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> DiffusionTensorEditorTest1
  ${CMAKE_CURRENT_SOURCE_DIR}/DiffusionEditorTensorTest.tcl
  )
