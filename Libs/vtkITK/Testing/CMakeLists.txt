find_package(VTK REQUIRED)
if(VTK_FOUND)
   include(${VTK_USE_FILE})
else(VTK_FOUND)
   message(FATAL_ERROR "Cannot build without VTK")
endif(VTK_FOUND)

find_package( ITK )
if( ITK_FOUND )
  include(${ITK_USE_FILE})
else( ITK_FOUND )
  message( FATAL_ERROR "Cannot build without ITK" )
endif( ITK_FOUND )


############################################################################
# The test is a stand-alone executable.  However, the Slicer
# launcher is needed to set up shared library paths correctly.
############################################################################

set(VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
add_executable(VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
target_link_libraries(VTKITKVectorReader
  vtkITK)
add_test(
  NAME VTKITKVectorReader
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:VTKITKVectorReader>
    ${MRML_TEST_DATA_DIR}/fixed.nrrd
  )

set(VTKITKBSPLINETRANSFORM_SOURCE VTKITKBSplineTransform.cxx)
add_executable(VTKITKBSplineTransform ${VTKITKBSPLINETRANSFORM_SOURCE})
target_link_libraries(VTKITKBSplineTransform
  vtkITK)
add_test(
  NAME VTKITKBSplineTransform
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:VTKITKBSplineTransform> VTKITKBSplineTransform
  )
