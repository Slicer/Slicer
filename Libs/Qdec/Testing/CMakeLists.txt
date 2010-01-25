SET(KIT QdecLib)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  QdecContrastTest1.cxx
  QdecDataTableTest1.cxx
  QdecFactorTest1.cxx
  QdecGlmDesignTest1.cxx
  QdecGlmFitResultsTest1.cxx
  QdecGlmFitTest1.cxx
  QdecProjectTest1.cxx
  QdecSubjectTest1.cxx
  QdecUtilitiesTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

SET( QdecLib_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(QdecLib_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${QdecLib_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SIMPLE_TEST( QdecContrastTest1 )
# SIMPLE_TEST( QdecDataTableTest1 )
ADD_TEST (QdecDataTableTest1 ${LAUNCH_EXE} ${QdecLib_TESTS} QdecDataTableTest1 ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/qdec.table.dat)
SIMPLE_TEST( QdecFactorTest1 )
SIMPLE_TEST( QdecGlmDesignTest1 )
SIMPLE_TEST( QdecGlmFitResultsTest1 )
SIMPLE_TEST( QdecGlmFitTest1 )
SIMPLE_TEST( QdecProjectTest1 )
SIMPLE_TEST( QdecSubjectTest1 )
SIMPLE_TEST( QdecUtilitiesTest1 )

# install extra files
SET(testFiles qdec.table.dat gender.levels)
foreach(file
   ${testFiles}
   )
   configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/${file}
    COPYONLY
    )
endforeach(file) 
