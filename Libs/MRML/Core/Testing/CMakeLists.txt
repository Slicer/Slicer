set(KIT ${PROJECT_NAME})
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLArrayPlotNodeTest1.cxx
  vtkMRMLBSplineTransformNodeTest1.cxx
  vtkMRMLCameraNodeTest1.cxx
  vtkMRMLClipModelsNodeTest1.cxx
  vtkMRMLColorNodeTest1.cxx
  vtkMRMLColorTableNodeTest1.cxx
  vtkMRMLColorTableStorageNodeTest1.cxx
  vtkMRMLCrosshairNodeTest1.cxx
  vtkMRMLdGEMRICProceduralColorNodeTest1.cxx
  vtkMRMLDiffusionImageVolumeNodeTest1.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeDisplayNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNodeTest1.cxx
  vtkMRMLDiffusionWeightedVolumeNodeTest1.cxx
  vtkMRMLDisplayableNodeTest1.cxx
  vtkMRMLDisplayNodeTest1.cxx
  vtkMRMLDoubleArrayNodeTest1.cxx
  vtkMRMLFiducialListNodeTest1.cxx
  vtkMRMLFiducialListStorageNodeTest1.cxx
  vtkMRMLFreeSurferModelOverlayStorageNodeTest1.cxx
  vtkMRMLFreeSurferModelStorageNodeTest1.cxx
  vtkMRMLFreeSurferProceduralColorNodeTest1.cxx
  vtkMRMLGlyphableVolumeDisplayNodeTest1.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNodeTest1.cxx
  vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1.cxx
  vtkMRMLGridTransformNodeTest1.cxx
  vtkMRMLHierarchyNodeTest1.cxx
  vtkMRMLHierarchyNodeTest3.cxx
  vtkMRMLDisplayableHierarchyNodeTest1.cxx
  vtkMRMLDisplayableHierarchyNodeTest2.cxx
  vtkMRMLDisplayableHierarchyNodeTest3.cxx
  vtkMRMLInteractionNodeTest1.cxx
  vtkMRMLLabelMapVolumeDisplayNodeTest1.cxx
  vtkMRMLLayoutNodeTest1.cxx
  vtkMRMLLinearTransformNodeTest1.cxx
  vtkMRMLModelDisplayNodeTest1.cxx
  vtkMRMLModelHierarchyNodeTest1.cxx
  vtkMRMLModelNodeTest1.cxx
  vtkMRMLModelStorageNodeTest1.cxx
  vtkMRMLNodeTest1.cxx
  vtkMRMLNonlinearTransformNodeTest1.cxx
  vtkMRMLNRRDStorageNodeTest1.cxx
  vtkMRMLOrthogonalLinePlotNodeTest1.cxx
  vtkMRMLPETProceduralColorNodeTest1.cxx
  vtkMRMLPlotNodeTest1.cxx
  vtkMRMLProceduralColorNodeTest1.cxx
  vtkMRMLROIListNodeTest1.cxx
  vtkMRMLROINodeTest1.cxx
  vtkMRMLScalarVolumeDisplayNodeTest1.cxx
  vtkMRMLScalarVolumeNodeTest1.cxx
  vtkMRMLScalarVolumeNodeTest2.cxx
  vtkMRMLSceneAddSingletonTest.cxx
  vtkMRMLSceneBatchProcessTest.cxx
  vtkMRMLSceneImportTest.cxx
  vtkMRMLSceneImportIDConflictTest.cxx
  vtkMRMLSceneIDTest.cxx
  vtkMRMLSceneSnapshotNodeTest1.cxx
  vtkMRMLSceneTest1.cxx
  #vtkMRMLSceneTest2.cxx
  vtkMRMLSceneViewStorageNodeTest1.cxx
  vtkMRMLSceneViewNodeEventsTest.cxx
  vtkMRMLSceneViewNodeTest1.cxx
  vtkMRMLSelectionNodeTest1.cxx
  vtkMRMLSliceCompositeNodeTest1.cxx
  vtkMRMLSliceNodeTest1.cxx
  vtkMRMLSnapshotClipNodeTest1.cxx
  vtkMRMLStorableNodeTest1.cxx
  vtkMRMLStorageNodeTest1.cxx
  vtkMRMLTensorVolumeNodeTest1.cxx
  vtkMRMLTimeSeriesVolumeNodeTest1.cxx
  vtkMRMLTransformableNodeTest1.cxx
  vtkMRMLTransformNodeTest1.cxx
  vtkMRMLTransformStorageNodeTest1.cxx
  vtkMRMLUnstructuredGridDisplayNodeTest1.cxx
  vtkMRMLUnstructuredGridNodeTest1.cxx
  vtkMRMLUnstructuredGridStorageNodeTest1.cxx
  vtkMRMLVectorVolumeDisplayNodeTest1.cxx
  vtkMRMLVectorVolumeNodeTest1.cxx
  vtkMRMLViewNodeTest1.cxx
  vtkMRMLVolumeArchetypeStorageNodeTest1.cxx
  vtkMRMLVolumeDisplayNodeTest1.cxx
  vtkMRMLVolumeHeaderlessStorageNodeTest1.cxx
  vtkMRMLVolumeNodeTest1.cxx
  vtkMRMLXYPlotManagerNodeTest1.cxx
  vtkObserverManagerTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests} vtkMRMLSceneEventRecorder.cxx)
target_link_libraries(${KIT}CxxTests ${lib_name})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
endmacro()

SIMPLE_TEST( vtkMRMLArrayPlotNodeTest1 )
SIMPLE_TEST( vtkMRMLBSplineTransformNodeTest1 )
SIMPLE_TEST( vtkMRMLCameraNodeTest1 )
SIMPLE_TEST( vtkMRMLClipModelsNodeTest1 )
SIMPLE_TEST( vtkMRMLColorNodeTest1 )
SIMPLE_TEST( vtkMRMLColorTableNodeTest1 )
SIMPLE_TEST( vtkMRMLColorTableStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLCrosshairNodeTest1 )
SIMPLE_TEST( vtkMRMLdGEMRICProceduralColorNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionImageVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionWeightedVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionWeightedVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLDisplayableNodeTest1 )
SIMPLE_TEST( vtkMRMLDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLDoubleArrayNodeTest1 )
SIMPLE_TEST( vtkMRMLFiducialListNodeTest1 )
SIMPLE_TEST( vtkMRMLFiducialListStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLFreeSurferModelOverlayStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLFreeSurferModelStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLFreeSurferProceduralColorNodeTest1 )
SIMPLE_TEST( vtkMRMLGlyphableVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLGlyphableVolumeSliceDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1 )
SIMPLE_TEST( vtkMRMLGridTransformNodeTest1 )
SIMPLE_TEST( vtkMRMLHierarchyNodeTest1 )
SIMPLE_TEST( vtkMRMLHierarchyNodeTest3 )
SIMPLE_TEST( vtkMRMLDisplayableHierarchyNodeTest1 )
SIMPLE_TEST( vtkMRMLDisplayableHierarchyNodeTest2 )
SIMPLE_TEST( vtkMRMLDisplayableHierarchyNodeTest3 )
SIMPLE_TEST( vtkMRMLInteractionNodeTest1 )
SIMPLE_TEST( vtkMRMLLabelMapVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLLayoutNodeTest1 )
SIMPLE_TEST( vtkMRMLLinearTransformNodeTest1 )
SIMPLE_TEST( vtkMRMLModelDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLModelHierarchyNodeTest1 )
SIMPLE_TEST( vtkMRMLModelNodeTest1 )
SIMPLE_TEST( vtkMRMLModelStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLNodeTest1 )
SIMPLE_TEST( vtkMRMLNonlinearTransformNodeTest1 )
SIMPLE_TEST( vtkMRMLNRRDStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLOrthogonalLinePlotNodeTest1 )
SIMPLE_TEST( vtkMRMLPETProceduralColorNodeTest1 )
SIMPLE_TEST( vtkMRMLPlotNodeTest1 )
SIMPLE_TEST( vtkMRMLProceduralColorNodeTest1 )
SIMPLE_TEST( vtkMRMLROIListNodeTest1 )
SIMPLE_TEST( vtkMRMLROINodeTest1 )
SIMPLE_TEST( vtkMRMLScalarVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLScalarVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLScalarVolumeNodeTest2 )
SIMPLE_TEST( vtkMRMLSceneAddSingletonTest )
SIMPLE_TEST( vtkMRMLSceneBatchProcessTest )
SIMPLE_TEST( vtkMRMLSceneImportIDConflictTest )
SIMPLE_TEST( vtkMRMLSceneIDTest )
SIMPLE_TEST( vtkMRMLSceneSnapshotNodeTest1 )
SIMPLE_TEST( vtkMRMLSceneTest1 )
SIMPLE_TEST( vtkMRMLSceneViewNodeEventsTest )
SIMPLE_TEST( vtkMRMLSceneViewNodeTest1 )
SIMPLE_TEST( vtkMRMLSceneViewStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLSelectionNodeTest1 )
SIMPLE_TEST( vtkMRMLSliceCompositeNodeTest1 )
SIMPLE_TEST( vtkMRMLSliceNodeTest1 )
SIMPLE_TEST( vtkMRMLSnapshotClipNodeTest1 )
SIMPLE_TEST( vtkMRMLStorableNodeTest1 )
SIMPLE_TEST( vtkMRMLStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLTensorVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLTimeSeriesVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformableNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLUnstructuredGridDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLUnstructuredGridNodeTest1 )
SIMPLE_TEST( vtkMRMLUnstructuredGridStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLVectorVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLVectorVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLViewNodeTest1 )
SIMPLE_TEST( vtkMRMLVolumeArchetypeStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLVolumeDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLVolumeHeaderlessStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLVolumeNodeTest1 )
SIMPLE_TEST( vtkMRMLXYPlotManagerNodeTest1 )
SIMPLE_TEST( vtkObserverManagerTest1 )

macro(SIMPLE_TEST_WITH_SCENE TESTNAME SCENEFILENAME)
  add_test(
    NAME ${TESTNAME}_${SCENEFILENAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      ${CMAKE_CURRENT_SOURCE_DIR}/${SCENEFILENAME}
    )
endmacro()


set( ScenesToTest
  clipCube.mrml
  cube.mrml
  diffusionEditor.mrml
  fiducial_tract.mrml
  helixGlyphs.mrml
  helix_roi_tract.mrml
  hier_test.mrml
  multi_display.mrml
  name.mrml
  SlicePipeline.mrml
  unstruct_grid.mrml
  vol_and_cube_camera.mrml
  vol_and_cube.mrml
  vol.mrml
  volRender.mrml
  volScene.mrml
  )

foreach(SceneToTest ${ScenesToTest})
  #SIMPLE_TEST_WITH_SCENE( vtkMRMLSCeneTest2 ${SceneToTest} )
  SIMPLE_TEST_WITH_SCENE( vtkMRMLSceneImportTest ${SceneToTest} )
endforeach()

