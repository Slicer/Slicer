set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLCameraDisplayableManagerTest1.cxx
  vtkMRMLModelDisplayableManagerTest.cxx
  vtkMRMLModelSliceDisplayableManagerTest.cxx
  vtkMRMLThreeDReformatDisplayableManagerTest1.cxx
  vtkMRMLThreeDViewDisplayableManagerFactoryTest1.cxx
  vtkMRMLDisplayableManagerFactoriesTest1.cxx
  vtkMRMLSliceViewDisplayableManagerFactoryTest.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(DisplayableManagerTest_SRCS
  vtkMRMLTestThreeDViewDisplayableManager.cxx
  vtkMRMLTestSliceViewDisplayableManager.cxx
  vtkMRMLTestCustomDisplayableManager.cxx
  )

# Create the instantiator for DisplayableManager classes.
set(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${PROJECT_NAME}TestInstantiator"
  DisplayableManagerInstantiatorTest_SRCS
  "${DisplayableManagerTest_SRCS}"
  ""
  ${PROJECT_BINARY_DIR}
  "")

add_executable(${KIT}CxxTests
  ${Tests}
  ${DisplayableManagerTest_SRCS}
  ${DisplayableManagerInstantiatorTest_SRCS})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#
# Add all the tests
#
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
foreach(test ${TestsToRun})
  get_filename_component(TESTNAME ${test} NAME_WE)
  add_test(
      NAME ${TESTNAME}
      COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      -V Baseline/${TESTNAME}.png
      )
endforeach()

set_tests_properties(vtkMRMLCameraDisplayableManagerTest1 PROPERTIES RUN_SERIAL TRUE)
