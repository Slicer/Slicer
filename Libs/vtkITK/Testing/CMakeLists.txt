
############################################################################
# The test is a stand-alone executable.  However, the Slicer
# launcher is needed to set up shared library paths correctly.
############################################################################

set(VTKITKTESTVECTORREADER_SOURCE VTKITKVectorReader.cxx)
ctk_add_executable_utf8(VTKITKVectorReader ${VTKITKTESTVECTORREADER_SOURCE})
target_link_libraries(VTKITKVectorReader PRIVATE Slicer::ITK ITKFactoryRegistration)
target_include_directories(VTKITKVectorReader PRIVATE ${ITKFactoryRegistration_INCLUDE_DIRS})
set_target_properties(VTKITKVectorReader PROPERTIES FOLDER vtkITK)

ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
  NAME VTKITKVectorReader
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:VTKITKVectorReader>
    DATA{${CMAKE_CURRENT_SOURCE_DIR}/../../MRML/Core/Testing/TestData/fixed.nrrd}
  )

slicer_add_python_unittest(SCRIPT vtkITKArchetypeDiffusionTensorReaderFile.py)
slicer_add_python_unittest(SCRIPT vtkITKArchetypeDICOMSeriesFileList.py)
slicer_add_python_unittest(SCRIPT vtkITKArchetypeScalarReaderFile.py)
