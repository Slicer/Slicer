
############################################################################
# The test is a stand-alone executable.  However, the Slicer
# launcher is needed to set up shared library paths correctly.
############################################################################

set(VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
ctk_add_executable_utf8(VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
target_link_libraries(VTKITKVectorReader
  vtkITK)

set_target_properties(VTKITKVectorReader PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
  NAME VTKITKVectorReader
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:VTKITKVectorReader>
    DATA{${MRML_TEST_DATA_DIR}/fixed.nrrd}
  )

slicer_add_python_unittest(SCRIPT vtkITKArchetypeDiffusionTensorReaderFile.py)
slicer_add_python_unittest(SCRIPT vtkITKArchetypeScalarReaderFile.py)
