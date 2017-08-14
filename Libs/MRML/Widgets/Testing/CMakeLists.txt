set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(${KIT}Testing_ITK_COMPONENTS
  ITKCommon
  )
find_package(ITK 4.6 COMPONENTS ${${KIT}Testing_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN  "
  TESTING_OUTPUT_INIT();
  itk::itkFactoryRegistration();
  ")

set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);
  DEBUG_LEAKS_ENABLE_EXIT_ERROR();
  ")

set(TEST_SOURCES
  qMRMLCheckableNodeComboBoxTest.cxx
  qMRMLCheckableNodeComboBoxTest1.cxx
  qMRMLClipNodeWidgetTest1.cxx
  qMRMLColorListViewTest1.cxx
  qMRMLColorModelTest.cxx
  qMRMLColorModelTest1.cxx
  qMRMLColorPickerWidgetTest1.cxx
  qMRMLColorPickerWidgetTest2.cxx
  qMRMLColorPickerWidgetTest3.cxx
  qMRMLColorTableComboBoxTest1.cxx
  qMRMLColorTableViewTest1.cxx
  qMRMLDisplayNodeViewComboBoxTest.cxx
  qMRMLDisplayNodeWidgetTest1.cxx
  qMRMLLayoutManagerTest1.cxx
  qMRMLLayoutManagerTest2.cxx
  qMRMLLayoutManagerTest3.cxx
  qMRMLLayoutManagerTest4.cxx
  qMRMLLayoutManagerVisibilityTest.cxx
  qMRMLLayoutManagerWithCustomFactoryTest.cxx
  qMRMLLinearTransformSliderTest1.cxx
  qMRMLListWidgetTest1.cxx
  qMRMLMatrixWidgetTest1.cxx
  qMRMLModelInfoWidgetTest1.cxx
  qMRMLModelTest1.cxx
  qMRMLModelTreeViewTest1.cxx
  qMRMLLabelComboBoxTest.cxx
  qMRMLNavigationViewTest1.cxx
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  qMRMLNodeComboBoxTest1.cxx
  qMRMLNodeComboBoxTest2.cxx
  qMRMLNodeComboBoxTest3.cxx
  qMRMLNodeComboBoxTest4.cxx
  qMRMLNodeComboBoxTest5.cxx
  qMRMLNodeComboBoxTest6.cxx
  qMRMLNodeComboBoxTest7.cxx
  qMRMLNodeComboBoxTest8.cxx
  qMRMLNodeComboBoxTest9.cxx
  qMRMLNodeComboBoxLazyUpdateTest1.cxx
  qMRMLNodeFactoryTest1.cxx
  qMRMLScalarInvariantComboBoxTest1.cxx
  qMRMLSceneCategoryModelTest1.cxx
  qMRMLSceneColorTableModelTest1.cxx
  qMRMLSceneFactoryWidgetTest1.cxx
  qMRMLSceneHierarchyModelTest1.cxx
  qMRMLSceneModelTest.cxx
  qMRMLSceneModelTest1.cxx
  qMRMLSceneModelHierarchyModelTest1.cxx
  qMRMLSceneModelHierarchyModelTest2.cxx
  #qMRMLTransformProxyModelTest1.cxx
  qMRMLSceneTransformModelTest1.cxx
  qMRMLSceneTransformModelTest2.cxx
  qMRMLSceneDisplayableModelTest1.cxx
  qMRMLSceneDisplayableModelTest2.cxx
  qMRMLSliceControllerWidgetTest.cxx
  qMRMLSliceWidgetTest1.cxx
  qMRMLSliceWidgetTest2.cxx
  qMRMLTableViewTest1.cxx
  qMRMLTransformSlidersTest1.cxx
  qMRMLThreeDViewTest1.cxx
  qMRMLThreeDWidgetTest1.cxx
  qMRMLTreeViewTest1.cxx
  qMRMLUtf8Test1.cxx
  qMRMLUtilsTest1.cxx
  qMRMLVolumeInfoWidgetTest1.cxx
  qMRMLVolumeThresholdWidgetTest1.cxx
  qMRMLVolumeThresholdWidgetTest2.cxx
  qMRMLWidgetsExportTest1.cxx
  qMRMLWindowLevelWidgetTest1.cxx
  EXTRA_INCLUDE qMRMLWidgetCxxTests.h 
  )

if(Slicer_USE_QtTesting)
  list(APPEND TEST_SOURCES
    qMRMLCheckableNodeComboBoxEventTranslatorPlayerTest1.cxx
    qMRMLClipNodeWidgetEventTranslatorPlayerTest1.cxx
    qMRMLCollapsibleButtonEventTranslatorPlayerTest1.cxx
    qMRMLColorListViewEventTranslatorPlayerTest1.cxx
    qMRMLColorPickerWidgetEventTranslatorPlayerTest1.cxx
    qMRMLColorTableComboBoxEventTranslatorPlayerTest1.cxx
    qMRMLColorTableViewEventTranslatorPlayerTest1.cxx
    qMRMLDisplayNodeWidgetEventTranslatorPlayerTest1.cxx
    qMRMLLabelComboBoxEventTranslatorPlayerTest1.cxx
    qMRMLLinearTransformSliderEventTranslatorPlayerTest1.cxx
    qMRMLListWidgetEventTranslatorPlayerTest1.cxx
    qMRMLMatrixWidgetEventTranslatorPlayerTest1.cxx
    qMRMLNavigationViewEventTranslatorPlayerTest1.cxx
    qMRMLNodeComboBoxEventTranslatorPlayerTest1.cxx
    qMRMLRangeWidgetEventTranslatorPlayerTest1.cxx
    qMRMLROIWidgetEventTranslatorPlayerTest1.cxx
    qMRMLScalarInvariantComboBoxEventTranslatorPlayerTest1.cxx
    qMRMLSliceControllerWidgetEventTranslatorPlayerTest1.cxx
    qMRMLSliceWidgetEventTranslatorPlayerTest1.cxx
    qMRMLThreeDViewControllerWidgetEventTranslatorPlayerTest1.cxx
    qMRMLThreeDWidgetEventTranslatorPlayerTest1.cxx
    qMRMLTransformSlidersEventTranslatorPlayerTest1.cxx
    qMRMLTreeViewEventTranslatorPlayerTest1.cxx
    qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1.cxx
    qMRMLVolumeThresholdWidgetEventTranslatorPlayerTest1.cxx
    qMRMLWindowLevelWidgetEventTranslatorPlayerTest1.cxx
    )
endif()

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

set(Tests_MOC_SRCS qMRMLNodeObject.h)
set(Tests_UtilityFiles
  qMRMLNodeObject.cxx
  qMRMLNodeObject.h
  )

set(KIT_TEST_GENERATE_MOC_SRCS
  qMRMLCheckableNodeComboBoxTest.cxx
  qMRMLColorModelTest.cxx
  qMRMLDisplayNodeViewComboBoxTest.cxx
  qMRMLLabelComboBoxTest.cxx
  qMRMLLayoutManagerWithCustomFactoryTest.cxx
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  qMRMLSceneModelTest.cxx
  qMRMLSliceControllerWidgetTest.cxx
  )
if(CTK_QT_VERSION VERSION_LESS "5")
  set(_moc_options)
  if(MRML_WIDGETS_HAVE_WEBKIT_SUPPORT)
    set(_moc_options OPTIONS -DMRML_WIDGETS_HAVE_WEBKIT_SUPPORT)
  endif()
  QT4_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS} ${_moc_options})
  QT4_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
else()
  set(_moc_options OPTIONS -DMRML_WIDGETS_HAVE_QT5)
  if(MRML_WIDGETS_HAVE_WEBKIT_SUPPORT)
    set(_moc_options OPTIONS -DMRML_WIDGETS_HAVE_WEBKIT_SUPPORT)
  endif()
  QT5_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS} ${_moc_options})
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
endif()

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

macro(SCENE_TEST TESTNAME SCENEFILENAME)
  add_test(
    NAME ${TESTNAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
    )
endmacro()

simple_test( qMRMLCheckableNodeComboBoxTest )
simple_test( qMRMLCheckableNodeComboBoxTest1 )
simple_test( qMRMLClipNodeWidgetTest1 )
simple_test( qMRMLColorListViewTest1 )
simple_test( qMRMLColorModelTest )
simple_test( qMRMLColorModelTest1 )
simple_test( qMRMLColorPickerWidgetTest1 )
simple_test( qMRMLColorPickerWidgetTest2 )
simple_test( qMRMLColorPickerWidgetTest3 )
simple_test( qMRMLColorTableComboBoxTest1 )
simple_test( qMRMLColorTableViewTest1 )
simple_test( qMRMLDisplayNodeViewComboBoxTest )
simple_test( qMRMLDisplayNodeWidgetTest1 )
simple_test( qMRMLLayoutManagerTest1 )
simple_test( qMRMLLayoutManagerTest2 )
simple_test( qMRMLLayoutManagerTest3 )
simple_test( qMRMLLayoutManagerTest4 )
simple_test( qMRMLLayoutManagerVisibilityTest )
simple_test( qMRMLLayoutManagerWithCustomFactoryTest )
simple_test( qMRMLLinearTransformSliderTest1 )
simple_test( qMRMLListWidgetTest1 )
simple_test( qMRMLMatrixWidgetTest1 )
simple_test( qMRMLModelInfoWidgetTest1 )
simple_test( qMRMLModelTest1 )
simple_test( qMRMLModelTreeViewTest1 )
simple_test( qMRMLLabelComboBoxTest )
simple_test( qMRMLNavigationViewTest1 )
simple_test( qMRMLNodeAttributeTableViewTest )
simple_test( qMRMLNodeAttributeTableWidgetTest )
simple_test( qMRMLNodeComboBoxTest1 )
simple_test( qMRMLNodeComboBoxTest2 )
SCENE_TEST(  qMRMLNodeComboBoxTest3 diffusionEditor.mrml )
simple_test( qMRMLNodeComboBoxTest4 )
simple_test( qMRMLNodeComboBoxTest5 )
simple_test( qMRMLNodeComboBoxTest6 )
simple_test( qMRMLNodeComboBoxTest7 )
simple_test( qMRMLNodeComboBoxTest8 )
simple_test( qMRMLNodeComboBoxTest9 )
simple_test( qMRMLNodeComboBoxLazyUpdateTest1 )
simple_test( qMRMLNodeFactoryTest1 )
simple_test( qMRMLScalarInvariantComboBoxTest1 )
simple_test( qMRMLSceneCategoryModelTest1 )
simple_test( qMRMLSceneColorTableModelTest1 )
simple_test( qMRMLSceneFactoryWidgetTest1 )
simple_test( qMRMLSceneModelTest )
simple_test( qMRMLSceneModelTest1 )
simple_test( qMRMLSceneModelHierarchyModelTest1 )
SCENE_TEST( qMRMLSceneModelHierarchyModelTest2 vol_and_cube.mrml)
simple_test( qMRMLSceneTransformModelTest1 )
SCENE_TEST(  qMRMLSceneTransformModelTest2 vol_and_cube.mrml )
simple_test( qMRMLSceneDisplayableModelTest1 )
SCENE_TEST(  qMRMLSceneDisplayableModelTest2 vol_and_cube.mrml )
simple_test( qMRMLSliceControllerWidgetTest )
SCENE_TEST( qMRMLSliceWidgetTest1 vol_and_cube.mrml)
SCENE_TEST( qMRMLSliceWidgetTest2 TestData/fixed.nrrd)
simple_test( qMRMLTableViewTest1 )
simple_test( qMRMLTransformSlidersTest1 )
simple_test( qMRMLThreeDViewTest1 )
simple_test( qMRMLThreeDWidgetTest1 )
SCENE_TEST(  qMRMLTreeViewTest1 vol_and_cube.mrml )
SCENE_TEST(  qMRMLUtf8Test1 cube-utf8.mrml )
simple_test( qMRMLUtilsTest1 )
simple_test( qMRMLVolumeInfoWidgetTest1 )
SCENE_TEST( qMRMLVolumeThresholdWidgetTest1 vol_and_cube.mrml)
SCENE_TEST( qMRMLVolumeThresholdWidgetTest2 vol_and_cube.mrml)
simple_test( qMRMLWidgetsExportTest1 )
SCENE_TEST( qMRMLWindowLevelWidgetTest1 vol_and_cube.mrml)

if(Slicer_USE_QtTesting)
  macro(QtTesting_TEST testname)
    simple_test(${testname} ${ARGN})
    set_tests_properties(${testname} PROPERTIES RUN_SERIAL ON)
  endmacro()
  QtTesting_TEST(qMRMLCheckableNodeComboBoxEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLClipNodeWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLCollapsibleButtonEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLColorListViewEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLColorPickerWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLColorTableComboBoxEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLColorTableViewEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLDisplayNodeWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLLabelComboBoxEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLLinearTransformSliderEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLListWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLMatrixWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLNavigationViewEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLNodeComboBoxEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLRangeWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLROIWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLScalarInvariantComboBoxEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLSliceControllerWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLSliceWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR} "${MRMLCore_SOURCE_DIR}/Testing/vol_and_cube.mrml")
  QtTesting_TEST(qMRMLThreeDViewControllerWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLThreeDWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLTransformSlidersEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLTreeViewEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR} "${MRMLCore_SOURCE_DIR}/Testing/vol_and_cube.mrml")
  QtTesting_TEST(qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR})
  QtTesting_TEST(qMRMLVolumeThresholdWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR} "${MRMLCore_SOURCE_DIR}/Testing/vol_and_cube.mrml")
  QtTesting_TEST(qMRMLWindowLevelWidgetEventTranslatorPlayerTest1 ${Slicer_SOURCE_DIR} "${MRMLCore_SOURCE_DIR}/Testing/vol_and_cube.mrml")
endif()
