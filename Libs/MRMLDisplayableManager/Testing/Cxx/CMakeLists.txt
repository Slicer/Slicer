SET(KIT MRMLDisplayableManager)

SET( KIT_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(KIT_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${KIT_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkMRMLCameraDisplayableManagerTest1.cxx
  vtkMRMLThreeDViewDisplayableManagerFactoryTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
)

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})
ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

set(TEMP "${Slicer3_BINARY_DIR}/Testing/Temporary")

#SIMPLE_TEST( vtkMRMLCameraDisplayableManagerTest1 )

#
# Add all the tests
#
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  ADD_TEST(${TName} ${LAUNCH_EXE} ${CXX_TEST_PATH}/${KIT}CxxTests ${TName}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      -V Baseline/${TName}.png
      )
ENDFOREACH()
