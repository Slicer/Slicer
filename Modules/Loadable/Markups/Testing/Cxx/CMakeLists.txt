set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(TEMP ${Slicer_BINARY_DIR}/Testing/Temporary)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLMarkupsDisplayNodeTest1.cxx
  vtkMRMLMarkupsFiducialNodeTest1.cxx
  vtkMRMLMarkupsNodeTest1.cxx
  vtkMRMLMarkupsNodeTest2.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest1.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest2.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest3.cxx
  vtkMRMLMarkupsStorageNodeTest1.cxx
  vtkSlicerMarkupsLogicTest1.cxx
  vtkSlicerMarkupsLogicTest2.cxx
  vtkSlicerMarkupsLogicTest3.cxx
  vtkMarkupsAnnotationSceneTest.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES
    vtkSlicerAnnotationsModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

SIMPLE_TEST( vtkMRMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsFiducialNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest2 )

SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest1 ${TEMP}/markupsFiducialStorageNode.fcsv )

# test legacy Slicer3 fcsv file
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest2 ${INPUT}/slicer3.fcsv )

# test Slicer4 annotation acsv file
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest3 ${INPUT}/slicer4.acsv )

SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest1 )

# logic tests
SIMPLE_TEST( vtkSlicerMarkupsLogicTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest2 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest3 )

# test Slicer4 annotation fiducials in a mrml file
# TODO: remove this after annotation fiducials have been removed
SIMPLE_TEST( vtkMarkupsAnnotationSceneTest ${INPUT}/AnnotationTest/AnnotationFiducialsTest.mrml )
