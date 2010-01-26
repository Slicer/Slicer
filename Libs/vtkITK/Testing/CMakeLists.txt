FIND_PACKAGE(VTK REQUIRED)
IF (VTK_FOUND)
   INCLUDE(${VTK_USE_FILE})
ELSE (VTK_FOUND)
   MESSAGE (FATAL_ERROR "Cannot build without VTK")
ENDIF (VTK_FOUND)

FIND_PACKAGE ( ITK )
IF ( ITK_FOUND )
  INCLUDE(${ITK_USE_FILE})
ELSE ( ITK_FOUND )
  MESSAGE ( FATAL_ERROR "Cannot build without ITK" )
ENDIF ( ITK_FOUND )


############################################################################
# The test is a stand-alone executable.  However, the Slicer3
# launcher is needed to set up shared library paths correctly.
############################################################################ 
SET( Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

SET( WRAPPED_EXE_PREFIX ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
IF (WIN32)
  SET( WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
ENDIF (WIN32)

SET (VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
ADD_EXECUTABLE (VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
TARGET_LINK_LIBRARIES (VTKITKVectorReader 
  vtkITK)
ADD_TEST (VTKITKVectorReader 
  ${Slicer3_EXE} --launch ${WRAPPED_EXE_PREFIX}/VTKITKVectorReader ${MRML_TEST_DATA_DIR}/fixed.nrrd
  )

SET (VTKITKBSPLINETRANSFORM_SOURCE VTKITKBSplineTransform.cxx)
ADD_EXECUTABLE (VTKITKBSplineTransform ${VTKITKBSPLINETRANSFORM_SOURCE})
TARGET_LINK_LIBRARIES (VTKITKBSplineTransform
  vtkITK)
ADD_TEST (VTKITKBSplineTransform
  ${Slicer3_EXE} --launch ${WRAPPED_EXE_PREFIX}/VTKITKBSplineTransform)
