set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(TEMP ${Slicer_BINARY_DIR}/Testing/Temporary)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

#-----------------------------------------------------------------------------
slicer_is_loadable_builtin_module_enabled("SceneViews" _build_scene_views_module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLMarkupsDisplayNodeTest1.cxx
  vtkMRMLMarkupsFiducialNodeTest1.cxx
  vtkMRMLMarkupsNodeTest1.cxx
  vtkMRMLMarkupsNodeTest2.cxx
  vtkMRMLMarkupsNodeTest3.cxx
  vtkMRMLMarkupsNodeTest4.cxx
  vtkMRMLMarkupsNodeTest5.cxx
  vtkMRMLMarkupsNodeTest6.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest2.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest3.cxx
  vtkMRMLMarkupsStorageNodeTest1.cxx
  vtkMRMLMarkupsStorageNodeTest2.cxx
  vtkSlicerMarkupsLogicTest1.cxx
  vtkSlicerMarkupsLogicTest2.cxx
  vtkSlicerMarkupsLogicTest3.cxx
  vtkSlicerMarkupsLogicTest4.cxx
  vtkMRMLMarkupsNodeEventsTest.cxx
  )
if(_build_scene_views_module)
  list(APPEND KIT_TEST_SRCS
    vtkMarkupsAnnotationSceneTest.cxx
    )
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(KIT_TEST_INCLUDE_DIRS
  )

if(_build_scene_views_module)
  list(APPEND KIT_TEST_INCLUDE_DIRS
    ${vtkSlicerSceneViewsModuleLogic_SOURCE_DIR}
    ${vtkSlicerSceneViewsModuleLogic_BINARY_DIR}
    )
endif()

#-----------------------------------------------------------------------------
set(KIT_TEST_TARGET_LIBRARIES
  vtkSlicerAnnotationsModuleLogic
  )

if(_build_scene_views_module)
  list(APPEND KIT_TEST_TARGET_LIBRARIES
    vtkSlicerSceneViewsModuleLogic
    )
endif()

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES ${KIT_TEST_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES ${KIT_TEST_INCLUDE_DIRS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

SIMPLE_TEST( vtkMRMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsFiducialNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest2 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest3 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest4 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest5 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest6 )
SIMPLE_TEST( vtkMRMLMarkupsNodeEventsTest )

# test legacy Slicer3 fcsv file
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest2 ${INPUT}/slicer3.fcsv )

# test Slicer4 annotation acsv file
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest3 ${INPUT}/slicer4.acsv )

SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest2 ${TEMP} )

# logic tests
SIMPLE_TEST( vtkSlicerMarkupsLogicTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest2 ${TEMP} )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest3 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest4 )

# test Slicer4 annotation fiducials in a mrml file
if(_build_scene_views_module)
  SIMPLE_TEST( vtkMarkupsAnnotationSceneTest ${INPUT}/AnnotationTest/AnnotationFiducialsTest.mrml )
endif()
