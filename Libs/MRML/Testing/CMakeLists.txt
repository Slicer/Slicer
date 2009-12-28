SET(KIT MRMLLib)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkMRMLColorNodeTest1.cxx
  vtkMRMLColorTableNodeTest1.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1.cxx
  vtkMRMLDisplayableNodeTest1.cxx
  vtkMRMLModelStorageNodeTest1.cxx
  vtkMRMLNodeTest1.cxx
  vtkMRMLSceneTest1.cxx
  vtkMRMLSceneTest2.cxx
  vtkMRMLStorableNodeTest1.cxx
  vtkMRMLStorageNodeTest1.cxx
  vtkMRMLTransformableNodeTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${lib_name})

SET( MRMLLib_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${LAUNCH_EXE} ${MRMLLib_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


SIMPLE_TEST( vtkMRMLColorNodeTest1 )
SIMPLE_TEST( vtkMRMLColorTableNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1 )
SIMPLE_TEST( vtkMRMLDisplayableNodeTest1 )
SIMPLE_TEST( vtkMRMLModelStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLNodeTest1 )
SIMPLE_TEST( vtkMRMLSceneTest1 )
SIMPLE_TEST( vtkMRMLStorableNodeTest1 )
SIMPLE_TEST( vtkMRMLStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformableNodeTest1 )

MACRO( TEST_SCENE  SCENENAME )
ADD_TEST( vtkMRMLSceneTest2_${SCENENAME} 
  ${MRMLLib_TESTS} vtkMRMLSceneTest2
  ${CMAKE_CURRENT_SOURCE_DIR}/${SCENENAME}
  )
ENDMACRO( TEST_SCENE )

SET( ScenesToTest 
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

FOREACH( SceneToTest ${ScenesToTest} )
  TEST_SCENE( ${SceneToTest} )
ENDFOREACH( SceneToTest )

