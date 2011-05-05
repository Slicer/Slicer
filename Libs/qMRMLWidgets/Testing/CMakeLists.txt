
SET(KIT qMRMLWidgets)
SET(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  qMRMLClipNodeWidgetTest1.cxx
  qMRMLColorListViewTest1.cxx
  qMRMLColorModelTest1.cxx
  qMRMLColorModelTest2.cxx
  qMRMLColorPickerWidgetTest1.cxx
  qMRMLColorPickerWidgetTest2.cxx
  qMRMLColorTableComboBoxTest1.cxx
  qMRMLColorTableViewTest1.cxx
  qMRMLDisplayNodeWidgetTest1.cxx
  qMRMLLayoutManagerTest1.cxx
  qMRMLLayoutManagerTest2.cxx
  qMRMLLayoutManagerTest3.cxx
  qMRMLLayoutManagerTest4.cxx
  qMRMLLinearTransformSliderTest1.cxx
  qMRMLListWidgetTest1.cxx
  qMRMLMatrixWidgetTest1.cxx
  qMRMLModelTest1.cxx
  qMRMLModelInfoWidgetTest1.cxx
  qMRMLLabelComboBoxTest1.cxx
  qMRMLNavigationViewTest1.cxx
  qMRMLNodeComboBoxTest1.cxx
  qMRMLNodeComboBoxTest2.cxx
  qMRMLNodeComboBoxTest3.cxx
  qMRMLNodeComboBoxTest4.cxx
  qMRMLNodeComboBoxTest5.cxx
  qMRMLNodeComboBoxTest6.cxx
  qMRMLNodeFactoryTest1.cxx
  qMRMLScalarInvariantComboBoxTest1.cxx
  qMRMLSceneCategoryModelTest1.cxx
  qMRMLSceneColorTableModelTest1.cxx
  qMRMLSceneFactoryWidgetTest1.cxx
  qMRMLSceneModelTest1.cxx
  qMRMLSceneModelHierarchyModelTest1.cxx
  #qMRMLTransformProxyModelTest1.cxx
  qMRMLSceneTransformModelTest1.cxx
  qMRMLSceneTransformModelTest2.cxx
  qMRMLSceneDisplayableModelTest1.cxx
  qMRMLSceneDisplayableModelTest2.cxx
  qMRMLSliceWidgetTest1.cxx
  qMRMLSliceWidgetTest2.cxx
  qMRMLTransformSlidersTest1.cxx
  qMRMLThreeDViewTest1.cxx
  qMRMLTreeViewTest1.cxx
  qMRMLUtilsTest1.cxx
  qMRMLVolumeInfoWidgetTest1.cxx
  qMRMLVolumeThresholdWidgetTest1.cxx
  qMRMLVolumeThresholdWidgetTest2.cxx
  qMRMLWidgetsExportTest1.cxx
  qMRMLWindowLevelWidgetTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET(Tests_MOC_SRCS qMRMLNodeObject.h)
SET(Tests_UtilityFiles 
    qMRMLNodeObject.cxx
    qMRMLNodeObject.h
    )

QT4_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS})

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

MACRO(SIMPLE_TEST TESTNAME)
  ADD_TEST(NAME ${TESTNAME} COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME})
ENDMACRO()

MACRO(SCENE_TEST TESTNAME SCENEFILENAME)
  ADD_TEST(
    NAME ${TESTNAME}_${SCENENAME} 
    COMMAND ${LAUNCH_EXE} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${MRML_SOURCE_DIR}/Testing/${SCENEFILENAME}
    )
ENDMACRO()

SIMPLE_TEST( qMRMLClipNodeWidgetTest1 )
SIMPLE_TEST( qMRMLColorListViewTest1 )
SIMPLE_TEST( qMRMLColorModelTest1 )
SIMPLE_TEST( qMRMLColorModelTest2 )
SIMPLE_TEST( qMRMLColorPickerWidgetTest1 )
SIMPLE_TEST( qMRMLColorPickerWidgetTest2 )
SIMPLE_TEST( qMRMLColorTableComboBoxTest1 )
SIMPLE_TEST( qMRMLColorTableViewTest1 )
SIMPLE_TEST( qMRMLDisplayNodeWidgetTest1 )
SIMPLE_TEST( qMRMLLayoutManagerTest1 )
SIMPLE_TEST( qMRMLLayoutManagerTest2 )
SIMPLE_TEST( qMRMLLayoutManagerTest3 )
SIMPLE_TEST( qMRMLLayoutManagerTest4 )
SIMPLE_TEST( qMRMLLinearTransformSliderTest1 )
SIMPLE_TEST( qMRMLListWidgetTest1 )
SIMPLE_TEST( qMRMLMatrixWidgetTest1 )
SIMPLE_TEST( qMRMLModelTest1 )
SIMPLE_TEST( qMRMLModelInfoWidgetTest1 )
SIMPLE_TEST( qMRMLLabelComboBoxTest1 )
SIMPLE_TEST( qMRMLNavigationViewTest1 )
SIMPLE_TEST( qMRMLNodeComboBoxTest1 )
SIMPLE_TEST( qMRMLNodeComboBoxTest2 )
SCENE_TEST(  qMRMLNodeComboBoxTest3 diffusionEditor.mrml )
SIMPLE_TEST( qMRMLNodeComboBoxTest4 )
SIMPLE_TEST( qMRMLNodeComboBoxTest5 )
SIMPLE_TEST( qMRMLNodeComboBoxTest6 )
SIMPLE_TEST( qMRMLNodeFactoryTest1 )
SIMPLE_TEST( qMRMLScalarInvariantComboBoxTest1 )
SIMPLE_TEST( qMRMLSceneCategoryModelTest1 )
SIMPLE_TEST( qMRMLSceneColorTableModelTest1 )
SIMPLE_TEST( qMRMLSceneFactoryWidgetTest1 )
SIMPLE_TEST( qMRMLSceneModelTest1 )
SIMPLE_TEST( qMRMLSceneModelHierarchyModelTest1 )
SIMPLE_TEST( qMRMLSceneTransformModelTest1 )
SCENE_TEST(  qMRMLSceneTransformModelTest2 vol_and_cube.mrml )
SIMPLE_TEST( qMRMLSceneDisplayableModelTest1 )
SCENE_TEST(  qMRMLSceneDisplayableModelTest2 vol_and_cube.mrml )
SCENE_TEST( qMRMLSliceWidgetTest1 vol_and_cube.mrml)
SCENE_TEST( qMRMLSliceWidgetTest2 fixed.nrrd)
SIMPLE_TEST( qMRMLTransformSlidersTest1 )
SIMPLE_TEST( qMRMLThreeDViewTest1 )
SCENE_TEST(  qMRMLTreeViewTest1 vol_and_cube.mrml )
SIMPLE_TEST( qMRMLUtilsTest1 )
SIMPLE_TEST( qMRMLVolumeInfoWidgetTest1 )
SCENE_TEST( qMRMLVolumeThresholdWidgetTest1 vol_and_cube.mrml)
SCENE_TEST( qMRMLVolumeThresholdWidgetTest2 vol_and_cube.mrml)
SIMPLE_TEST( qMRMLWidgetsExportTest1 )
SCENE_TEST( qMRMLWindowLevelWidgetTest1 vol_and_cube.mrml)
