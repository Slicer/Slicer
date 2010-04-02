SET(KIT qCTKWidgets)
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qCTKAbstractFactoryTest1.cxx
  qCTKAbstractLibraryFactoryTest1.cxx
  qCTKAbstractObjectFactoryTest1.cxx
  qCTKAbstractPluginFactoryTest1.cxx
  qCTKAbstractQObjectFactoryTest1.cxx
  qCTKAddRemoveComboBoxTest1.cxx
  qCTKButtonGroupTest1.cxx
  qCTKCollapsibleButtonTest1.cxx
  qCTKCollapsibleGroupBoxTest1.cxx
  qCTKColorPickerButtonTest1.cxx
  qCTKCoordinatesWidgetTest1.cxx
  qCTKDoubleRangeSliderTest1.cxx
  qCTKDoubleSliderTest1.cxx
  qCTKFittedTextBrowserTest1.cxx
  qCTKFlowLayoutTest1.cxx
  qCTKMatrixWidgetTest1.cxx
  qCTKModelTesterTest1.cxx
  qCTKRangeSliderTest1.cxx
  qCTKPimplTest1.cxx
  qCTKSingletonTest1.cxx
  qCTKSliderSpinBoxWidgetTest1.cxx
  qCTKTitleComboBoxTest1.cxx
  qCTKTreeComboBoxTest1.cxx
  qCTKUtilsTest1.cxx
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( qCTKWidgets_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

IF(WIN32)
  SET(qCTKWidgets_TESTS ${CXX_TEST_PATH}/${CMAKE_BUILD_TYPE}/${KIT}CxxTests)
ENDIF(WIN32)


MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE}  ${qCTKWidgets_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )

SIMPLE_TEST( qCTKAbstractFactoryTest1 )
SIMPLE_TEST( qCTKAbstractLibraryFactoryTest1 )
SIMPLE_TEST( qCTKAbstractObjectFactoryTest1 )
SIMPLE_TEST( qCTKAbstractPluginFactoryTest1 )
SIMPLE_TEST( qCTKAbstractQObjectFactoryTest1 )
SIMPLE_TEST( qCTKAddRemoveComboBoxTest1 )
SIMPLE_TEST( qCTKButtonGroupTest1 )
SIMPLE_TEST( qCTKCollapsibleButtonTest1 )
SIMPLE_TEST( qCTKCollapsibleGroupBoxTest1 )
SIMPLE_TEST( qCTKColorPickerButtonTest1 )
SIMPLE_TEST( qCTKCoordinatesWidgetTest1 )
SIMPLE_TEST( qCTKDoubleRangeSliderTest1 )
SIMPLE_TEST( qCTKDoubleSliderTest1 )
SIMPLE_TEST( qCTKFittedTextBrowserTest1 )
SIMPLE_TEST( qCTKFlowLayoutTest1 )
SIMPLE_TEST( qCTKMatrixWidgetTest1 )
SIMPLE_TEST( qCTKModelTesterTest1 )
SIMPLE_TEST( qCTKPimplTest1 )
SIMPLE_TEST( qCTKRangeSliderTest1 )
SIMPLE_TEST( qCTKSingletonTest1 )
SIMPLE_TEST( qCTKSliderSpinBoxWidgetTest1 )
SIMPLE_TEST( qCTKTitleComboBoxTest1 )
SIMPLE_TEST( qCTKTreeComboBoxTest1 )
SIMPLE_TEST( qCTKUtilsTest1 )
