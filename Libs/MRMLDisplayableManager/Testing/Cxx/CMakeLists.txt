SET(KIT MRMLDisplayableManager)

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkMRMLCameraDisplayableManagerTest1.cxx
  vtkMRMLThreeDViewDisplayableManagerFactoryTest1.cxx
  vtkMRMLDisplayableManagerFactoriesTest1.cxx
  vtkMRMLSliceModelDisplayableManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
)

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

SET(DisplayableManagerTest_SRCS
  vtkMRMLTestThreeDViewDisplayableManager.cxx
  vtkMRMLTestSliceViewDisplayableManager.cxx
  vtkMRMLTestCustomDisplayableManager.cxx
  )

# Create the instantiator for DisplayableManager classes.
SET(VTK_USE_INSTANTIATOR_NEW 1)
VTK_MAKE_INSTANTIATOR3("${PROJECT_NAME}TestInstantiator"
  DisplayableManagerInstantiatorTest_SRCS
  "${DisplayableManagerTest_SRCS}"
  ""
  ${PROJECT_BINARY_DIR}
  "")

SET(LIBRARY_NAME ${PROJECT_NAME})
ADD_EXECUTABLE(${KIT}CxxTests
  ${Tests}
  ${DisplayableManagerTest_SRCS}
  ${DisplayableManagerInstantiatorTest_SRCS})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

#MACRO(SIMPLE_TEST TESTNAME)
#  ADD_TEST(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
#ENDMACRO()

#
# Add all the tests
#
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TESTNAME ${test} NAME_WE)
  ADD_TEST(
      NAME ${TESTNAME} 
      COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      -V Baseline/${TESTNAME}.png
      )
ENDFOREACH()
