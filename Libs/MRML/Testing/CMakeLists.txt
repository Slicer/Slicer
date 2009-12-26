SET(KIT MRMLLib)
CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx
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


SIMPLE_TEST( vtkMRMLDisplayableNodeTest1 )
SIMPLE_TEST( vtkMRMLModelStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLNodeTest1 )
SIMPLE_TEST( vtkMRMLStorableNodeTest1 )
SIMPLE_TEST( vtkMRMLStorageNodeTest1 )
SIMPLE_TEST( vtkMRMLTransformableNodeTest1 )
