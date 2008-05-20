project(MimxTesting)

include_regular_expression("^.*$")

include_directories( ${MimxMeshing_SOURCE_DIR}/Filter 
  ${MimxMeshing_BINARY_DIR}/Filter 
  ${MimxMeshing_SOURCE_DIR}/IO 
  ${MimxMeshing_BINARY_DIR}/IO) 

if( NOT DISABLE_CXX_TESTING )
  if(ITK_BINARY_DIR)
    if(TCL_TCLSH)
      add_test(PrintSelf-BasicFilters ${TCL_TCLSH}
        ${ITK_SOURCE_DIR}/Utilities/Dart/PrintSelfCheck.tcl  
        ${ITK_SOURCE_DIR}/Code/BasicFilters)
    endif(TCL_TCLSH)
  endif(ITK_BINARY_DIR)

  ############################################################################
  # The test is a stand-alone executable.  However, the Slicer3
  # launcher is needed to set up shared library paths correctly.
  # We create a prefix for the test executable that accomplishes this
  # wrapping.
  ############################################################################
  set( Slicer3_EXE ${Slicer3_BINARY_DIR}/Slicer3 )
  set( WRAPPED_EXE_PREFIX --launch ${EXECUTABLE_OUTPUT_PATH})
  if(WIN32)
    set(WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
  endif(WIN32)

  add_executable(vtkRebinMaterialPropertyTest vtkRebinMaterialPropertyTest.cxx)
  target_link_libraries(vtkRebinMaterialPropertyTest mimxFilter
    vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
    ITKIO ITKBasicFilters ITKCommon itkvnl_algo itkvnl)
  add_test(vtkRebinMaterialPropertyTest ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/vtkRebinMaterialPropertyTest)


  add_executable(itkImageToVtkUnstructuredGridTest itkImageToVtkUnstructuredGridTest.cxx)
  target_link_libraries(itkImageToVtkUnstructuredGridTest mimxFilter 
    vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
    ITKIO ITKBasicFilters ITKCommon itkvnl_algo itkvnl)
  add_test(itkImageToVtkUnstructuredGridTest ${Slicer3_EXE} ${WRAPPED_EXE_PREFIX}/itkImageToVtkUnstructuredGridTest)



endif( NOT DISABLE_CXX_TESTING )
