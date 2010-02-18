SET(KIT qVTKWidgets)

#
# Testing data
#
# Resource file used for testing is generated running:
#    bin/qVTKWidgetsCxxTests qVTKCommandOptionsTest1 --generate-test-data

SET(TESTING_DATA ${CMAKE_CURRENT_SOURCE_DIR}/Data)
SET(QTESTVTKCOMMANDOPTIONS_SETTINGS ${TESTING_DATA}/Settings/qVTKCommandOptionsTest1.ini)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/qVTKWidgetsCxxTestsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qVTKWidgetsCxxTestsConfigure.h
  )

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

#
# Create test
#

CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qVTKCommandOptionsTest1.cxx
  qVTKObjectTest1.cxx
  )

SET(qVTKWidgetsTest_SRCS
  qVTKObjectTest.cxx
  )

SET(qVTKWidgetsTest_MOC_SRCS
  qVTKObjectTest.h
  )

QT4_WRAP_CPP(qVTKWidgetsTest_SRCS ${qVTKWidgetsTest_MOC_SRCS})

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests} ${qVTKWidgetsTest_SRCS})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( qVTKWidgets_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)
IF(WIN32)
  SET(qVTKWidgets_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${qVTKWidgets_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

#SIMPLE_TEST( qVTKCommandOptionsTest1 )

ADD_TEST( qVTKCommandOptionsTest1 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --help )
          
ADD_TEST( qVTKCommandOptionsTest2 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --enable-awesome-test )

ADD_TEST( qVTKCommandOptionsTest3 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 -eat )

ADD_TEST( qVTKCommandOptionsTest3 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --disable-settings --test-if-disablesettings-flag-works )

SET(arg1 "Testing with")
SET(arg2 " cmake is ")
SET(arg3 "super awesome !")
SET(arg4 "CMake rocks !")

IF(WIN32)
ADD_TEST( qVTKCommandOptionsTest4 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --my-list-of-strings "${arg1}" "${arg2}" "${arg3}"
          -ms="${arg4}" --test-if-commandline-overwrite-settings )

ADD_TEST( qVTKCommandOptionsTest5 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 -mlos "${arg1}" "${arg2}" "${arg3}"
          --my-string="${arg4}" --test-if-commandline-overwrite-settings )
ELSE()
ADD_TEST( qVTKCommandOptionsTest4 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --my-list-of-strings ${arg1} ${arg2} ${arg3}
          -ms=${arg4} --test-if-commandline-overwrite-settings )

ADD_TEST( qVTKCommandOptionsTest5 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 -mlos ${arg1} ${arg2} ${arg3}
          --my-string=${arg4} --test-if-commandline-overwrite-settings )
ENDIF()

ADD_TEST( qVTKCommandOptionsTest6 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
          qVTKCommandOptionsTest1 --test-if-ignorerest-flag-works
          --ignore-rest --nothing=foo extra1 /? )

#ADD_TEST( qVTKCommandOptionsTest7 ${LAUNCH_EXE} ${qVTKWidgets_TESTS}
#          qVTKCommandOptionsTest1 --test-if-ignorerest-flag-works
#          -- --nothing=foo extra1 /? )

# qVTKObjectTest1
SIMPLE_TEST( qVTKObjectTest1 )
