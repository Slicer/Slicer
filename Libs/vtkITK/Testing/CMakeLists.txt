
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

slicer_add_python_unittest(SCRIPT vtkITKArchetypeDiffusionTensorReaderFile.py)
slicer_add_python_unittest(SCRIPT vtkITKArchetypeScalarReaderFile.py)
