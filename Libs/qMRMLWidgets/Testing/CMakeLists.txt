
SET(KIT qMRMLWidgets)
SET(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qMRMLItemHelperTest1.cxx
  qMRMLLinearTransformSliderTest1.cxx
  qMRMLListWidgetTest1.cxx
  qMRMLMatrixWidgetTest1.cxx
  qMRMLModelTest1.cxx
  qMRMLNodeFactoryButtonTest1.cxx
  qMRMLNodeFactoryTest1.cxx
  qMRMLNodeSelectorTest1.cxx
  qMRMLNodeTreeSelectorTest1.cxx
  qMRMLSceneGeneratorButtonTest1.cxx
  qMRMLSceneModelTest1.cxx
  qMRMLTransformProxyModelTest1.cxx
  qMRMLTransformSlidersTest1.cxx
  qMRMLTreeWidgetTest1.cxx
  qMRMLUtilsTest1.cxx
  qMRMLWidgetsExportTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( qMRMLWidgets_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)
IF(WIN32)
  SET(qMRMLWidgets_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${qMRMLWidgets_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SIMPLE_TEST( qMRMLItemHelperTest1 )
SIMPLE_TEST( qMRMLLinearTransformSliderTest1 )
SIMPLE_TEST( qMRMLListWidgetTest1 )
SIMPLE_TEST( qMRMLMatrixWidgetTest1 )
SIMPLE_TEST( qMRMLModelTest1 )
SIMPLE_TEST( qMRMLNodeFactoryButtonTest1 )
SIMPLE_TEST( qMRMLNodeFactoryTest1 )
SIMPLE_TEST( qMRMLNodeSelectorTest1 )
SIMPLE_TEST( qMRMLNodeTreeSelectorTest1 )
SIMPLE_TEST( qMRMLSceneGeneratorButtonTest1 )
SIMPLE_TEST( qMRMLSceneModelTest1 )
SIMPLE_TEST( qMRMLTransformProxyModelTest1 )
SIMPLE_TEST( qMRMLTransformSlidersTest1 )
SIMPLE_TEST( qMRMLTreeWidgetTest1 )
SIMPLE_TEST( qMRMLUtilsTest1 )
SIMPLE_TEST( qMRMLWidgetsExportTest1 )
