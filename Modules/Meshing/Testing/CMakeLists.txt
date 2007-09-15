PROJECT (MimxTesting)

INCLUDE_REGULAR_EXPRESSION("^.*$")

INCLUDE_DIRECTORIES( ${MimxMeshing_SOURCE_DIR}/Filter 
                     ${MimxMeshing_BINARY_DIR}/Filter 
                     ${MimxMeshing_SOURCE_DIR}/IO 
                     ${MimxMeshing_BINARY_DIR}/IO) 

IF( NOT DISABLE_CXX_TESTING )
IF(ITK_BINARY_DIR)
  IF(TCL_TCLSH)
    ADD_TEST(PrintSelf-BasicFilters ${TCL_TCLSH}
             ${ITK_SOURCE_DIR}/Utilities/Dart/PrintSelfCheck.tcl  
             ${ITK_SOURCE_DIR}/Code/BasicFilters)
  ENDIF(TCL_TCLSH)
ENDIF(ITK_BINARY_DIR)


# Configure Dart testing support.  This should be done before any
# MESSAGE(FATAL_ERROR ...) commands are invoked.
INCLUDE(${CMAKE_ROOT}/Modules/Dart.cmake)
ENABLE_TESTING()



ADD_EXECUTABLE(vtkRebinMaterialPropertyTest vtkRebinMaterialPropertyTest.cxx)
TARGET_LINK_LIBRARIES(vtkRebinMaterialPropertyTest mimxFilter
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
                      ITKIO ITKBasicFilters ITKCommon itkvnl_algo itkvnl)
ADD_TEST(vtkRebinMaterialPropertyTest ${EXECUTABLE_OUTPUT_PATH}/vtkRebinMaterialPropertyTest)


ADD_EXECUTABLE(itkImageToVtkUnstructuredGridTest itkImageToVtkUnstructuredGridTest.cxx)
TARGET_LINK_LIBRARIES(itkImageToVtkUnstructuredGridTest mimxFilter 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
                      ITKIO ITKBasicFilters ITKCommon itkvnl_algo itkvnl)
ADD_TEST(itkImageToVtkUnstructuredGridTest ${EXECUTABLE_OUTPUT_PATH}/itkImageToVtkUnstructuredGridTest)



ENDIF( NOT DISABLE_CXX_TESTING )
