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
# The test is a stand-alone executable.  However, the Slicer
# launcher is needed to set up shared library paths correctly.
############################################################################ 

SET (VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
ADD_EXECUTABLE (VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
TARGET_LINK_LIBRARIES (VTKITKVectorReader 
  vtkITK)
ADD_TEST (
  NAME VTKITKVectorReader 
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:VTKITKVectorReader>
    ${MRML_TEST_DATA_DIR}/fixed.nrrd
  )

SET (VTKITKBSPLINETRANSFORM_SOURCE VTKITKBSplineTransform.cxx)
ADD_EXECUTABLE (VTKITKBSplineTransform ${VTKITKBSPLINETRANSFORM_SOURCE})
TARGET_LINK_LIBRARIES (VTKITKBSplineTransform
  vtkITK)
ADD_TEST (
  NAME VTKITKBSplineTransform
  COMMAND ${LAUNCH_EXE} $<TARGET_FILE:VTKITKBSplineTransform> VTKITKBSplineTransform
  )
