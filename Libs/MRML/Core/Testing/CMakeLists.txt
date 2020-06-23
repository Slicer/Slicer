set(KIT ${PROJECT_NAME})

set(EXTRA_INCLUDE "vtkMRMLDebugLeaksMacro.h\"\n\#include \"vtkTestingOutputWindow.h")
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkMRMLBSplineTransformNodeTest1.cxx
  vtkMRMLCameraNodeTest1.cxx
  vtkMRMLClipModelsNodeTest1.cxx
  vtkMRMLColorNodeTest1.cxx
  vtkMRMLColorTableNodeTest1.cxx
  vtkMRMLColorTableStorageNodeTest1.cxx
  vtkMRMLCoreTestingUtilitiesTest.cxx
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
  vtkMRMLPlotChartNodeTest1.cxx
  vtkMRMLPlotSeriesNodeTest1.cxx
  vtkMRMLPlotViewNodeTest1.cxx
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
  vtkMRMLSceneTest2.cxx
  vtkMRMLSceneDefaultNodeTest.cxx
  # Disabled scene view tests for now - they will be fixed in upcoming commit
  # vtkMRMLSceneViewNodeImportSceneTest.cxx
  # vtkMRMLSceneViewNodeEventsTest.cxx
  # vtkMRMLSceneViewNodeRestoreSceneTest.cxx
  # vtkMRMLSceneViewNodeStoreSceneTest.cxx
  vtkMRMLSceneViewNodeTest1.cxx
  vtkMRMLSceneViewStorageNodeTest1.cxx
  vtkMRMLScriptedModuleNodeTest1.cxx
  vtkMRMLSegmentationStorageNodeTest1.cxx
  vtkMRMLSelectionNodeTest1.cxx
  vtkMRMLSliceCompositeNodeTest1.cxx
  vtkMRMLSliceNodeTest1.cxx
  vtkMRMLSnapshotClipNodeTest1.cxx
  vtkMRMLStorableNodeTest1.cxx
  vtkMRMLStorageNodeTest1.cxx
  vtkMRMLStreamingVolumeNodeTest1.cxx
  vtkMRMLTableNodeTest1.cxx
  vtkMRMLTableStorageNodeTest1.cxx
  vtkMRMLTableSQLiteStorageNodeTest.cxx
  vtkMRMLTableViewNodeTest1.cxx
  vtkMRMLTensorVolumeNodeTest1.cxx
  vtkMRMLTextNodeTest1.cxx
  vtkMRMLTextStorageNodeTest1.cxx
  vtkMRMLTransformableNodeReferenceSaveImportTest.cxx
  vtkMRMLTransformableNodeOnNodeReferenceAddTest.cxx
  vtkMRMLTransformDisplayNodeTest1.cxx
  vtkMRMLTransformNodeTest1.cxx
  vtkMRMLTransformStorageNodeTest1.cxx
  vtkMRMLTransformableNodeTest1.cxx
  vtkMRMLUnitNodeTest1.cxx
  vtkMRMLVectorVolumeDisplayNodeTest1.cxx
  vtkMRMLVectorVolumeNodeTest1.cxx
  vtkMRMLViewNodeTest1.cxx
  vtkMRMLVolumeArchetypeStorageNodeTest1.cxx
  vtkMRMLVolumeDisplayNodeTest1.cxx
  vtkMRMLVolumeHeaderlessStorageNodeTest1.cxx
  vtkMRMLVolumeNodeEventsTest.cxx
  vtkMRMLVolumeNodeTest1.cxx
  vtkMRMLdGEMRICProceduralColorNodeTest1.cxx
  vtkArchiveTest1.cxx
  vtkCodedEntryTest1.cxx
  vtkObserverManagerTest1.cxx
  vtkOrientedBSplineTransformTest1.cxx
  vtkOrientedGridTransformTest1.cxx
  vtkThinPlateSplineTransformTest1.cxx
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} vtkMRMLSceneEventRecorder.cxx)
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
set(DATAPATH "${CMAKE_CURRENT_SOURCE_DIR}/TestData")
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test( vtkMRMLBSplineTransformNodeTest1 )
simple_test( vtkMRMLCameraNodeTest1 )
simple_test( vtkMRMLClipModelsNodeTest1 )
simple_test( vtkMRMLColorNodeTest1 )
simple_test( vtkMRMLColorTableNodeTest1 ${TEMP})
simple_test( vtkMRMLColorTableStorageNodeTest1 )
simple_test( vtkMRMLCoreTestingUtilitiesTest )
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
simple_test( vtkMRMLDoubleArrayNodeTest1 ${TEMP} ${DATAPATH})
simple_test( vtkMRMLFiducialListNodeTest1 )
simple_test( vtkMRMLFiducialListStorageNodeTest1 )
simple_test( vtkMRMLGlyphableVolumeDisplayNodeTest1 )
simple_test( vtkMRMLGlyphableVolumeSliceDisplayNodeTest1 )
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
simple_test( vtkMRMLModelStorageNodeTest1 ${TEMP})
simple_test( vtkMRMLNodeTest1 )
simple_test( vtkMRMLLinearTransformNodeEventsTest )
simple_test( vtkMRMLNonlinearTransformNodeTest1 ${CMAKE_CURRENT_SOURCE_DIR}/NonLinearTransformScene.mrml)
simple_test( vtkMRMLNRRDStorageNodeTest1 )
simple_test( vtkMRMLPETProceduralColorNodeTest1 )
simple_test( vtkMRMLPlotChartNodeTest1 )
simple_test( vtkMRMLPlotSeriesNodeTest1 )
simple_test( vtkMRMLPlotViewNodeTest1 )
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
simple_test( vtkMRMLSceneDefaultNodeTest )
# Disabled scene view tests for now - they will be fixed in upcoming commit
# simple_test( vtkMRMLSceneViewNodeImportSceneTest )
# simple_test( vtkMRMLSceneViewNodeEventsTest )
# simple_test( vtkMRMLSceneViewNodeRestoreSceneTest )
# simple_test( vtkMRMLSceneViewNodeStoreSceneTest )
simple_test( vtkMRMLSceneViewNodeTest1 )
simple_test( vtkMRMLSceneViewStorageNodeTest1 )
simple_test( vtkMRMLSegmentationStorageNodeTest1
  DATA{${INPUT}/ITKSnapSegmentation.nii.gz}
  DATA{${INPUT}/OldSlicerSegmentation.seg.nrrd}
  DATA{${INPUT}/SlicerSegmentation.seg.nrrd}
  )
simple_test( vtkMRMLSelectionNodeTest1 )
simple_test( vtkMRMLSliceCompositeNodeTest1 )
simple_test( vtkMRMLSliceNodeTest1 )
simple_test( vtkMRMLSnapshotClipNodeTest1 )
simple_test( vtkMRMLStorableNodeTest1 )
simple_test( vtkMRMLStorageNodeTest1 )
simple_test( vtkMRMLStreamingVolumeNodeTest1 )
simple_test( vtkMRMLTableNodeTest1 )
simple_test( vtkMRMLTableStorageNodeTest1 ${TEMP})
simple_test( vtkMRMLTableViewNodeTest1 )
simple_test( vtkMRMLTensorVolumeNodeTest1 )
simple_test( vtkMRMLTextNodeTest1 )
simple_test( vtkMRMLTextStorageNodeTest1 ${TEMP})
simple_test( vtkMRMLTransformableNodeReferenceSaveImportTest )
simple_test( vtkMRMLTransformableNodeOnNodeReferenceAddTest )
simple_test( vtkMRMLTransformableNodeTest1 )
simple_test( vtkMRMLTransformDisplayNodeTest1 )
simple_test( vtkMRMLTransformNodeTest1 )
simple_test( vtkMRMLTransformStorageNodeTest1 )
simple_test( vtkMRMLUnitNodeTest1 )
simple_test( vtkMRMLVectorVolumeDisplayNodeTest1 )
simple_test( vtkMRMLVectorVolumeNodeTest1 )
simple_test( vtkMRMLViewNodeTest1 )
simple_test( vtkMRMLVolumeArchetypeStorageNodeTest1 )
simple_test( vtkMRMLVolumeDisplayNodeTest1 )
simple_test( vtkMRMLVolumeHeaderlessStorageNodeTest1 )
simple_test( vtkMRMLVolumeNodeEventsTest )
simple_test( vtkMRMLVolumeNodeTest1 )
simple_test( vtkArchiveTest1 DATA{${INPUT}/vol.zip} )
simple_test( vtkCodedEntryTest1 )
simple_test( vtkObserverManagerTest1 )
simple_test( vtkOrientedBSplineTransformTest1 )
simple_test( vtkOrientedGridTransformTest1 )
simple_test( vtkThinPlateSplineTransformTest1 )

function(SIMPLE_TEST_WITH_SCENE TESTNAME SCENEFILENAME)
  # Extract list of external files to download. Note that the ${_externalfiles} variable
  # is only specified to trigger download of data files used in the scene, the arguments
  # are NOT explcitly used in ${KIT}CxxTests test executable.
  string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
  list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
  list(LENGTH scenefilename_and_externalfiles _length)
  if(_length EQUAL 2)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${SCENEFILENAME}
      ${CMAKE_CURRENT_BINARY_DIR}/${SCENEFILENAME}
      COPYONLY
      )
    set(_scenefile ${CMAKE_CURRENT_BINARY_DIR}/${SCENEFILENAME})
    list(GET scenefilename_and_externalfiles 1 _externalfiles)
  else()
    set(_scenefile ${CMAKE_CURRENT_SOURCE_DIR}/${SCENEFILENAME})
    set(_externalfiles "")
  endif()
  simple_test(${TESTNAME}_${SCENEFILENAME} DRIVER_TESTNAME ${TESTNAME} ${_scenefile} ${_externalfiles})
endfunction()

set( ScenesToTest
  clipCube.mrml
  cube.mrml
  diffusionEditor.mrml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz,helix-DWI.nhdr,helix-DWI.raw.gz}
  helixGlyphs.mrml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz}
  hier_test.mrml
  multi_display.mrml|DATA{${INPUT}/cube.vtk}
  name.mrml|DATA{${INPUT}/cube.vtk,fixed.nrrd,moving.nrrd}
  SlicePipeline.mrml|DATA{${INPUT}/fixed.nrrd}
  volumetric_mesh.mrml
  vol_and_cube_camera.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk}
  vol_and_cube.mrml|DATA{${INPUT}/fixed.nrrd,cube.vtk}
  vol.mrml|DATA{${INPUT}/fixed.nrrd}
  volScene.mrml|DATA{${INPUT}/fixed.nrrd,moving.nrrd}
  )

foreach(SceneToTest ${ScenesToTest})
  SIMPLE_TEST_WITH_SCENE( vtkMRMLSceneTest2 ${SceneToTest} )
  SIMPLE_TEST_WITH_SCENE( vtkMRMLSceneImportTest ${SceneToTest} )
endforeach()
