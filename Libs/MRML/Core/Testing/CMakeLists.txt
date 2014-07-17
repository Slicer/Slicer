set(KIT ${PROJECT_NAME})
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLBSplineTransformNodeTest1.cxx
  vtkMRMLCameraNodeTest1.cxx
  vtkMRMLClipModelsNodeTest1.cxx
  vtkMRMLColorNodeTest1.cxx
  vtkMRMLColorTableNodeTest1.cxx
  vtkMRMLColorTableStorageNodeTest1.cxx
  vtkMRMLCrosshairNodeTest1.cxx
  vtkMRMLDiffusionImageVolumeNodeTest1.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeDisplayNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeNodeTest1.cxx
  vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1.cxx
  vtkMRMLDiffusionWeightedVolumeDisplayNodeTest1.cxx
  vtkMRMLDiffusionWeightedVolumeNodeTest1.cxx
  vtkMRMLDisplayNodeTest1.cxx
  vtkMRMLDisplayableHierarchyNodeDisplayPropertiesTest.cxx
  vtkMRMLDisplayableHierarchyNodeTest1.cxx
  vtkMRMLDisplayableHierarchyNodeTest2.cxx
  vtkMRMLDisplayableHierarchyNodeTest3.cxx
  vtkMRMLDisplayableNodeTest1.cxx
  vtkMRMLDoubleArrayNodeTest1.cxx
  vtkMRMLFiducialListNodeTest1.cxx
  vtkMRMLFiducialListStorageNodeTest1.cxx
  vtkMRMLFreeSurferModelOverlayStorageNodeTest1.cxx
  vtkMRMLFreeSurferModelStorageNodeTest1.cxx
  vtkMRMLFreeSurferProceduralColorNodeTest1.cxx
  vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1.cxx
  vtkMRMLGlyphableVolumeDisplayNodeTest1.cxx
  vtkMRMLGlyphableVolumeSliceDisplayNodeTest1.cxx
  vtkMRMLGridTransformNodeTest1.cxx
  vtkMRMLHierarchyNodeTest1.cxx
  vtkMRMLHierarchyNodeTest3.cxx
  vtkMRMLInteractionNodeTest1.cxx
  vtkMRMLLabelMapVolumeDisplayNodeTest1.cxx
  vtkMRMLLayoutNodeTest1.cxx
  vtkMRMLLinearTransformNodeEventsTest.cxx
  vtkMRMLLinearTransformNodeTest1.cxx
  vtkMRMLModelDisplayNodeTest1.cxx
  vtkMRMLModelHierarchyNodeTest1.cxx
  vtkMRMLModelNodeTest1.cxx
  vtkMRMLModelStorageNodeTest1.cxx
  vtkMRMLNRRDStorageNodeTest1.cxx
  vtkMRMLNodeTest1.cxx
  vtkMRMLNonlinearTransformNodeTest1.cxx
  vtkMRMLPETProceduralColorNodeTest1.cxx
  vtkMRMLProceduralColorNodeTest1.cxx
  vtkMRMLProceduralColorStorageNodeTest1.cxx
  vtkMRMLROIListNodeTest1.cxx
  vtkMRMLROINodeTest1.cxx
  vtkMRMLScalarVolumeDisplayNodeTest1.cxx
  vtkMRMLScalarVolumeNodeTest1.cxx
  vtkMRMLScalarVolumeNodeTest2.cxx
  vtkMRMLSceneAddSingletonTest.cxx
  vtkMRMLSceneBatchProcessTest.cxx
  vtkMRMLSceneIDTest.cxx
  vtkMRMLSceneImportIDConflictTest.cxx
  vtkMRMLSceneImportIDModelHierarchyConflictTest.cxx
  vtkMRMLSceneImportIDModelHierarchyParentIDConflictTest.cxx
  vtkMRMLSceneImportTest.cxx
  vtkMRMLSceneTest1.cxx
  #vtkMRMLSceneTest2.cxx
  vtkMRMLSceneViewNodeImportSceneTest.cxx
  vtkMRMLSceneViewNodeEventsTest.cxx
  vtkMRMLSceneViewNodeRestoreSceneTest.cxx
  vtkMRMLSceneViewNodeStoreSceneTest.cxx
  vtkMRMLSceneViewNodeTest1.cxx
  vtkMRMLSceneViewStorageNodeTest1.cxx
  vtkMRMLSelectionNodeTest1.cxx
  vtkMRMLSliceCompositeNodeTest1.cxx
  vtkMRMLSliceNodeTest1.cxx
  vtkMRMLSnapshotClipNodeTest1.cxx
  vtkMRMLStorableNodeTest1.cxx
  vtkMRMLStorageNodeTest1.cxx
  vtkMRMLTensorVolumeNodeTest1.cxx
  vtkMRMLTransformableNodeReferenceSaveImportTest.cxx
  vtkMRMLTransformableNodeOnNodeReferenceAddTest.cxx
  vtkMRMLTransformDisplayNodeTest1.cxx
  vtkMRMLTransformNodeTest1.cxx
  vtkMRMLTransformStorageNodeTest1.cxx
  vtkMRMLTransformableNodeTest1.cxx
  vtkMRMLUnitNodeTest1.cxx
  vtkMRMLUnstructuredGridDisplayNodeTest1.cxx
  vtkMRMLUnstructuredGridNodeTest1.cxx
  vtkMRMLUnstructuredGridStorageNodeTest1.cxx
  vtkMRMLVectorVolumeDisplayNodeTest1.cxx
  vtkMRMLVectorVolumeNodeTest1.cxx
  vtkMRMLViewNodeTest1.cxx
  vtkMRMLVolumeArchetypeStorageNodeTest1.cxx
  vtkMRMLVolumeDisplayNodeTest1.cxx
  vtkMRMLVolumeHeaderlessStorageNodeTest1.cxx
  vtkMRMLVolumeNodeEventsTest.cxx
  vtkMRMLVolumeNodeTest1.cxx
  vtkMRMLdGEMRICProceduralColorNodeTest1.cxx
  vtkObserverManagerTest1.cxx
  vtkOrientedBSplineTransformTest1.cxx
  vtkOrientedGridTransformTest1.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

add_executable(${KIT}CxxTests ${Tests} vtkMRMLSceneEventRecorder.cxx)
target_link_libraries(${KIT}CxxTests ${KIT})

#-----------------------------------------------------------------------------
set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test( vtkMRMLBSplineTransformNodeTest1 )
simple_test( vtkMRMLCameraNodeTest1 )
simple_test( vtkMRMLClipModelsNodeTest1 )
simple_test( vtkMRMLColorNodeTest1 )
simple_test( vtkMRMLColorTableNodeTest1 ${TEMP})
simple_test( vtkMRMLColorTableStorageNodeTest1 )
simple_test( vtkMRMLCrosshairNodeTest1 )
simple_test( vtkMRMLdGEMRICProceduralColorNodeTest1 )
simple_test( vtkMRMLDiffusionImageVolumeNodeTest1 )
simple_test( vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1 )
simple_test( vtkMRMLDiffusionTensorVolumeDisplayNodeTest1 )
simple_test( vtkMRMLDiffusionTensorVolumeNodeTest1 )
simple_test( vtkMRMLDiffusionTensorVolumeSliceDisplayNodeTest1 )
simple_test( vtkMRMLDiffusionWeightedVolumeDisplayNodeTest1 )
simple_test( vtkMRMLDiffusionWeightedVolumeNodeTest1 )
simple_test( vtkMRMLDisplayableNodeTest1 )
simple_test( vtkMRMLDisplayNodeTest1 )
simple_test( vtkMRMLDoubleArrayNodeTest1 )
simple_test( vtkMRMLFiducialListNodeTest1 )
simple_test( vtkMRMLFiducialListStorageNodeTest1 )
simple_test( vtkMRMLFreeSurferModelOverlayStorageNodeTest1 )
simple_test( vtkMRMLFreeSurferModelStorageNodeTest1 )
simple_test( vtkMRMLFreeSurferProceduralColorNodeTest1 )
simple_test( vtkMRMLGlyphableVolumeDisplayNodeTest1 )
simple_test( vtkMRMLGlyphableVolumeSliceDisplayNodeTest1 )
simple_test( vtkMRMLGlyphVolumeDisplayPropertiesNodeTest1 )
simple_test( vtkMRMLGridTransformNodeTest1 )
simple_test( vtkMRMLHierarchyNodeTest1 )
simple_test( vtkMRMLHierarchyNodeTest3 )
simple_test( vtkMRMLDisplayableHierarchyNodeDisplayPropertiesTest )
simple_test( vtkMRMLDisplayableHierarchyNodeTest1 )
simple_test( vtkMRMLDisplayableHierarchyNodeTest2 )
simple_test( vtkMRMLDisplayableHierarchyNodeTest3 )
simple_test( vtkMRMLInteractionNodeTest1 )
simple_test( vtkMRMLLabelMapVolumeDisplayNodeTest1 )
simple_test( vtkMRMLLayoutNodeTest1 )
simple_test( vtkMRMLLinearTransformNodeTest1 )
simple_test( vtkMRMLModelDisplayNodeTest1 )
simple_test( vtkMRMLModelHierarchyNodeTest1 )
simple_test( vtkMRMLModelNodeTest1 )
simple_test( vtkMRMLModelStorageNodeTest1 )
simple_test( vtkMRMLNodeTest1 )
simple_test( vtkMRMLLinearTransformNodeEventsTest )
simple_test( vtkMRMLNonlinearTransformNodeTest1 ${CMAKE_CURRENT_SOURCE_DIR}/NonLinearTransformScene.mrml)
simple_test( vtkMRMLNRRDStorageNodeTest1 )
simple_test( vtkMRMLPETProceduralColorNodeTest1 )
simple_test( vtkMRMLProceduralColorNodeTest1 )
simple_test( vtkMRMLProceduralColorStorageNodeTest1 )
simple_test( vtkMRMLROIListNodeTest1 )
simple_test( vtkMRMLROINodeTest1 )
simple_test( vtkMRMLScalarVolumeDisplayNodeTest1 )
simple_test( vtkMRMLScalarVolumeNodeTest1 )
simple_test( vtkMRMLScalarVolumeNodeTest2 )
simple_test( vtkMRMLSceneAddSingletonTest )
simple_test( vtkMRMLSceneBatchProcessTest )
simple_test( vtkMRMLSceneImportIDConflictTest )
simple_test( vtkMRMLSceneImportIDModelHierarchyConflictTest )
simple_test( vtkMRMLSceneImportIDModelHierarchyParentIDConflictTest )
simple_test( vtkMRMLSceneIDTest )
simple_test( vtkMRMLSceneTest1 )
simple_test( vtkMRMLSceneViewNodeImportSceneTest )
simple_test( vtkMRMLSceneViewNodeEventsTest )
simple_test( vtkMRMLSceneViewNodeRestoreSceneTest )
simple_test( vtkMRMLSceneViewNodeStoreSceneTest )
simple_test( vtkMRMLSceneViewNodeTest1 )
simple_test( vtkMRMLSceneViewStorageNodeTest1 )
simple_test( vtkMRMLSelectionNodeTest1 )
simple_test( vtkMRMLSliceCompositeNodeTest1 )
simple_test( vtkMRMLSliceNodeTest1 )
simple_test( vtkMRMLSnapshotClipNodeTest1 )
simple_test( vtkMRMLStorableNodeTest1 )
simple_test( vtkMRMLStorageNodeTest1 )
simple_test( vtkMRMLTensorVolumeNodeTest1 )
simple_test( vtkMRMLTransformableNodeReferenceSaveImportTest )
simple_test( vtkMRMLTransformableNodeOnNodeReferenceAddTest )
simple_test( vtkMRMLTransformableNodeTest1 )
simple_test( vtkMRMLTransformDisplayNodeTest1 )
simple_test( vtkMRMLTransformNodeTest1 )
simple_test( vtkMRMLTransformStorageNodeTest1 )
simple_test( vtkMRMLUnitNodeTest1 )
simple_test( vtkMRMLUnstructuredGridDisplayNodeTest1 )
simple_test( vtkMRMLUnstructuredGridNodeTest1 )
simple_test( vtkMRMLUnstructuredGridStorageNodeTest1 )
simple_test( vtkMRMLVectorVolumeDisplayNodeTest1 )
simple_test( vtkMRMLVectorVolumeNodeTest1 )
simple_test( vtkMRMLViewNodeTest1 )
simple_test( vtkMRMLVolumeArchetypeStorageNodeTest1 )
simple_test( vtkMRMLVolumeDisplayNodeTest1 )
simple_test( vtkMRMLVolumeHeaderlessStorageNodeTest1 )
simple_test( vtkMRMLVolumeNodeTest1 )
simple_test( vtkObserverManagerTest1 )
simple_test( vtkOrientedBSplineTransformTest1 )

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

