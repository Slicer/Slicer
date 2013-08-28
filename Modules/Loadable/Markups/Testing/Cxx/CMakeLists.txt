set(KIT qSlicer${MODULE_NAME}Module)

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
  )


SET (TEST_DIR "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Markups/Testing")
# make sure the directory exists
MESSAGE(STATUS "Creating directory ${TEST_DIR}")
FILE(MAKE_DIRECTORY ${TEST_DIR} )

SIMPLE_TEST( vtkMRMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsFiducialNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest2 )


SET (TEST_FCSV ${TEST_DIR}/markupsFiducialStorageNode.fcsv)
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest1 ${TEST_FCSV} )


# test legacy Slicer3 fcsv file
SET (TEST_S3_FCSV ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/slicer3.fcsv)
MESSAGE(STATUS "Slicer 3 fiducials test: \nCMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}\n file for markups = ${TEST_S3_FCSV}")
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest2 ${TEST_S3_FCSV} )

# test Slicer4 annotation acsv file
SET (TEST_S4_ACSV ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/slicer4.acsv)
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest3 ${TEST_S4_ACSV} )

SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest1 )

# logic tests
SIMPLE_TEST( vtkSlicerMarkupsLogicTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest2 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest3 )

# test Slicer4 annotation fiducials in a mrml file
# TODO: enable this after final cut over
# SET (TEST_S4_ANNOTATION_MRML ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/AnnotationTest/AnnotationFiducialsTest.mrml)
# SIMPLE_TEST( vtkMarkupsAnnotationSceneTest ${TEST_S4_ANNOTATION_MRML} )
