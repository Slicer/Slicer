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
  qMRMLPlotViewTest1.cxx
  qMRMLScalarInvariantComboBoxTest1.cxx
  qMRMLSceneCategoryModelTest1.cxx
  qMRMLSceneColorTableModelTest1.cxx
  qMRMLSceneFactoryWidgetTest1.cxx
  qMRMLSceneHierarchyModelTest1.cxx
  qMRMLSceneModelTest.cxx
  qMRMLSceneModelTest1.cxx
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
  set(_moc_options OPTIONS -DMRML_WIDGETS_HAVE_QT5)
  QT5_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS} ${_moc_options})
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing/TestData)

#-----------------------------------------------------------------------------
function(SCENE_TEST TESTNAME SCENEFILENAME)
  # Extract list of external files to download. Note that the ${_externalfiles} variable
  # is only specified to trigger download of data files used in the scene, the arguments
  # are NOT explcitly used in ${KIT}CxxTests test executable.
  string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
  list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
  list(LENGTH scenefilename_and_externalfiles _length)
  if(_length EQUAL 2)
    configure_file(
      ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
      ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME}
      COPYONLY
      )
    set(_scenefile ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME})
    list(GET scenefilename_and_externalfiles 1 _externalfiles)
  else()
    set(_scenefile ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME})
    set(_externalfiles "")
  endif()
  simple_test(${TESTNAME}_${SCENEFILENAME} DRIVER_TESTNAME ${TESTNAME} ${_scenefile} ${_externalfiles})
endfunction()

#-----------------------------------------------------------------------------
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
SCENE_TEST(  qMRMLNodeComboBoxTest3 diffusionEditor.mrml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz,helix-DWI.nhdr,helix-DWI.raw.gz} )
simple_test( qMRMLNodeComboBoxTest4 )
simple_test( qMRMLNodeComboBoxTest5 )
simple_test( qMRMLNodeComboBoxTest6 )
simple_test( qMRMLNodeComboBoxTest7 )
simple_test( qMRMLNodeComboBoxTest8 )
simple_test( qMRMLNodeComboBoxTest9 )
simple_test( qMRMLNodeComboBoxLazyUpdateTest1 )
simple_test( qMRMLNodeFactoryTest1 )
simple_test( qMRMLPlotViewTest1 )
simple_test( qMRMLScalarInvariantComboBoxTest1 )
simple_test( qMRMLSceneCategoryModelTest1 )
simple_test( qMRMLSceneColorTableModelTest1 )
simple_test( qMRMLSceneFactoryWidgetTest1 )
simple_test( qMRMLSceneModelTest )
simple_test( qMRMLSceneModelTest1 )
simple_test( qMRMLSceneTransformModelTest1 )
SCENE_TEST(  qMRMLSceneTransformModelTest2 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
simple_test( qMRMLSceneDisplayableModelTest1 )
SCENE_TEST(  qMRMLSceneDisplayableModelTest2 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
simple_test( qMRMLSliceControllerWidgetTest )
SCENE_TEST( qMRMLSliceWidgetTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
simple_test( qMRMLSliceWidgetTest2_fixed.nrrd DRIVER_TESTNAME qMRMLSliceWidgetTest2 DATA{${INPUT}/fixed.nrrd})
simple_test( qMRMLTableViewTest1 )
simple_test( qMRMLTransformSlidersTest1 )
simple_test( qMRMLThreeDViewTest1 )
simple_test( qMRMLThreeDWidgetTest1 )
SCENE_TEST(  qMRMLTreeViewTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
SCENE_TEST(  qMRMLUtf8Test1 cube-utf8.mrml|DATA{${INPUT}/fixed.nrrd )
simple_test( qMRMLUtilsTest1 )
simple_test( qMRMLVolumeInfoWidgetTest1 )
SCENE_TEST( qMRMLVolumeThresholdWidgetTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
SCENE_TEST( qMRMLVolumeThresholdWidgetTest2 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
simple_test( qMRMLWidgetsExportTest1 )
SCENE_TEST( qMRMLWindowLevelWidgetTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})

if(Slicer_USE_QtTesting)
  function(QtTesting_TEST testname)
    if(${ARGC} GREATER 1)
      set(SCENEFILENAME ${ARGV1})

      # Extract list of external files to download. Note that the ${_externalfiles} variable
      # is only specified to trigger download of data files used in the scene, the arguments
      # are NOT explcitly used in ${KIT}CxxTests test executable.
      string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
      list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
      list(LENGTH scenefilename_and_externalfiles _length)
      if(_length EQUAL 2)
        configure_file(
          ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
          ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME}
          COPYONLY
          )
        set(_scenefile ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME})
        list(GET scenefilename_and_externalfiles 1 _externalfiles)
      else()
        set(_scenefile ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME})
        set(_externalfiles "")
      endif()

    else()
      set(_scenefile "")
    endif()
    simple_test(${testname} ${Slicer_SOURCE_DIR} ${_scenefile})
    set_tests_properties(${testname} PROPERTIES RUN_SERIAL ON)
  endfunction()
  QtTesting_TEST(qMRMLCheckableNodeComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLClipNodeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLCollapsibleButtonEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorListViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorPickerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorTableComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorTableViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLDisplayNodeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLLabelComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLLinearTransformSliderEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLListWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLMatrixWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLNavigationViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLNodeComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLRangeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLROIWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLScalarInvariantComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLSliceControllerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLSliceWidgetEventTranslatorPlayerTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qMRMLThreeDViewControllerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLThreeDWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLTransformSlidersEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLTreeViewEventTranslatorPlayerTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qMRMLVolumeInfoWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLVolumeThresholdWidgetEventTranslatorPlayerTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qMRMLWindowLevelWidgetEventTranslatorPlayerTest1 vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
endif()
