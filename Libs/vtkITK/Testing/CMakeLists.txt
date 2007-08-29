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
SET( SLICER_EXE ${Slicer3_BINARY_DIR}/Slicer3 )

SET (VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
ADD_EXECUTABLE (VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
TARGET_LINK_LIBRARIES (VTKITKVectorReader 
  vtkITK ITKCommon)
ADD_TEST (VTKITKVectorReader 
  ${SLICER_EXE} --launch ${EXECUTABLE_OUTPUT_PATH}/VTKITKVectorReader ${MRML_SOURCE_DIR}/Testing/TestData/fixed.nrrd)
