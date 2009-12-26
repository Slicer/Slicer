SET(KIT MRMLLib)
SET(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
  vtkMRMLColorNodeTest1.cxx
  vtkMRMLColorTableNodeTest1.cxx
  vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1.cxx
  vtkMRMLDisplayableNodeTest1.cxx
  vtkMRMLModelStorageNodeTest1.cxx
  vtkMRMLNodeTest1.cxx
  vtkMRMLStorableNodeTest1.cxx
  vtkMRMLStorageNodeTest1.cxx
  vtkMRMLTransformableNodeTest1.cxx
  EXTRA_INCLUDE TestingMacros.h
  )

SET (TestsToRun ${Tests})
REMOVE (TestsToRun ${KIT}CxxTests.cxx)

SET(LIBRARY_NAME ${PROJECT_NAME})

ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
TARGET_LINK_LIBRARIES(${KIT}CxxTests ${LIBRARY_NAME})

SET( MRMLLib_TESTS ${CXX_TEST_PATH}/${KIT}CxxTests)

MACRO( SIMPLE_TEST  TESTNAME )
  ADD_TEST( ${TESTNAME} ${MRMLLib_TESTS} ${TESTNAME} )
ENDMACRO( SIMPLE_TEST  )


SIMPLE_TEST( vtkMRMLColorNodeTest1 )
SIMPLE_TEST( vtkMRMLColorTableNodeTest1 )
SIMPLE_TEST( vtkMRMLDiffusionTensorDisplayPropertiesNodeTest1 )
SIMPLE_TEST( vtkMRMLDisplayableNodeTest1 )
SIMPLE_TEST( vtkMRMLModelStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLNodeTest1 )
SIMPLE_TEST( vtkMRMLStorableNodeTest1 )
SIMPLE_TEST( vtkMRMLStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformableNodeTest1 )
