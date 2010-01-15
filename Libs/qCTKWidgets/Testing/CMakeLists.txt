SET(KIT qCTKWidgets)
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qCTKModelTesterTest1
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( qCTKWidgets_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

ADD_TEST( qCTKModelTesterTest1 ${qCTKWidgets_TESTS} qCTKModelTesterTest1 )
