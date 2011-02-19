project(MimxTesting)

include_regular_expression("^.*$")

include_directories( ${MimxMeshing_SOURCE_DIR}/Filter 
  ${MimxMeshing_BINARY_DIR}/Filter 
  ${MimxMeshing_SOURCE_DIR}/Mesh
  ${MimxMeshing_BINARY_DIR}/Common 
  ${MimxMeshing_SOURCE_DIR}/Common
  ${MimxMeshing_SOURCE_DIR}/IO 
  ${MimxMeshing_BINARY_DIR}/IO) 

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
  set( Slicer_EXE ${Slicer_BINARY_DIR}/Slicer3 )
  set( WRAPPED_EXE_PREFIX --launch ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  if(WIN32)
    set(WRAPPED_EXE_PREFIX ${WRAPPED_EXE_PREFIX}/${CMAKE_BUILD_TYPE})
  endif(WIN32)

#  add_executable(vtkRebinMaterialPropertyTest vtkRebinMaterialPropertyTest.cxx)
#  target_link_libraries(vtkRebinMaterialPropertyTest mimxFilter
#    vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
#    ${ITK_LIBRARIES})
#  add_test(vtkRebinMaterialPropertyTest ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/vtkRebinMaterialPropertyTest)


  add_executable(itkImageToVtkUnstructuredGridTest itkImageToVtkUnstructuredGridTest.cxx)
  target_link_libraries(itkImageToVtkUnstructuredGridTest mimxFilter 
    vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon 
    ${ITK_LIBRARIES})
  add_test(MimxitkImageToVtkUnstructuredGridTest ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/itkImageToVtkUnstructuredGridTest)


  add_executable(MainAddUnstructuredHexahedronGridCellTest MainAddUnstructuredHexahedronGridCellTest.cxx)
  target_link_libraries(MainAddUnstructuredHexahedronGridCellTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxAddUnstructuredHexahedronGridCellTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainAddUnstructuredHexahedronGridCellTest
    ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk
    )

  add_executable(MainDeleteUnstructuredHexahedronGridCellTest MainDeleteUnstructuredHexahedronGridCellTest.cxx)
  target_link_libraries(MainDeleteUnstructuredHexahedronGridCellTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxDeleteUnstructuredHexahedronGridCellTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainDeleteUnstructuredHexahedronGridCellTest
    ${MimxTesting_SOURCE_DIR}/Data/BBox6.vtk
    )

  add_executable(MainMirrorUnstructuredHexahedronGridCellTest MainMirrorUnstructuredHexahedronGridCellTest.cxx)
  target_link_libraries(MainMirrorUnstructuredHexahedronGridCellTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxMirrorUnstructuredHexahedronGridCellTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainMirrorUnstructuredHexahedronGridCellTest
    ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk
    )

  add_executable(MainSplitUnstructuredHexahedronGridCellTest MainSplitUnstructuredHexahedronGridCellTest.cxx)
  target_link_libraries(MainSplitUnstructuredHexahedronGridCellTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxUnstructuredHexahedronGridCellTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainSplitUnstructuredHexahedronGridCellTest
    ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk
    )
############## Re-Enable when Meshing Integratation is completed
#  add_executable(MainApplyImageBasedMaterialPropertiesTest MainApplyImageBasedMaterialPropertiesTest.cxx )
#  target_link_libraries(MainApplyImageBasedMaterialPropertiesTest mimxFilter mimxCommon 
#                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
#  add_test(
#    MimxApplyImageBasedMaterialPropertiesTest
#    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainApplyImageBasedMaterialPropertiesTest
#    ${MimxTesting_SOURCE_DIR}/Data/CA05042124RFinal.hdr 
#    ${MimxTesting_SOURCE_DIR}/Data/CAfemesh.vtk
#    )
##############

  add_executable(MainBoundingBoxFromUnstructuredGridTest MainBoundingBoxFromUnstructuredGridTest.cxx )
  target_link_libraries(MainBoundingBoxFromUnstructuredGridTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxBoundingBoxFromUnstructuredGridTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainBoundingBoxFromUnstructuredGridTest
    ${MimxTesting_SOURCE_DIR}/Data/dividedbb.vtk 
    ${MimxTesting_SOURCE_DIR}/Data/dividedmorphsmooth.vtk
    )


  add_executable(MainEquivalancePointsTest MainEquivalancePointsTest.cxx )
  target_link_libraries(MainEquivalancePointsTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxEquivalancePointsTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainEquivalancePointsTest
    ${MimxTesting_SOURCE_DIR}/Data/Clubfoot_Link_38_Mesh.vtk
    )


  add_executable(MainExtractSurfaceTest MainExtractSurfaceTest.cxx )
  target_link_libraries(MainExtractSurfaceTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxExtractSurfaceTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainExtractSurfaceTest
    ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk
    )

  add_executable(MainBoundingBoxFromBoundsTest MainBoundingBoxFromBoundsTest.cxx )
  target_link_libraries(MainBoundingBoxFromBoundsTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxBoundingBoxFromBoundsTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainBoundingBoxFromBoundsTest
    ${MimxTesting_SOURCE_DIR}/Data/bbox2_modified.vtk
    )


  add_executable(MainExtractStructuredGridEdgeTest MainExtractStructuredGridEdgeTest.cxx )
  target_link_libraries(MainExtractStructuredGridEdgeTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxExtractStructuredGridEdgeTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainExtractStructuredGridEdgeTest
    ${MimxTesting_SOURCE_DIR}/Data/sgrid.vtk
    )

  add_executable(MainExtractStructuredGridFaceTest MainExtractStructuredGridFaceTest.cxx )
  target_link_libraries(MainExtractStructuredGridFaceTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxExtractStructuredGridFaceTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainExtractStructuredGridFaceTest
    ${MimxTesting_SOURCE_DIR}/Data/sgrid.vtk
    )


  add_executable(MainGeometryFilterTest MainGeometryFilterTest.cxx )
  target_link_libraries(MainGeometryFilterTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxGeometryFilterTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainGeometryFilterTest
    ${MimxTesting_SOURCE_DIR}/Data/Vert_Bod.vtk
    )

  add_executable(MainBoundingBoxToStructuredGridsTest MainBoundingBoxToStructuredGridsTest.cxx )
  target_link_libraries(MainBoundingBoxToStructuredGridsTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxBoundingBoxToStructuredGridsTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainBoundingBoxToStructuredGridsTest
    ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk
    )

  add_executable(MainMorphStructuredGridTest MainMorphStructuredGridTest.cxx )
  target_link_libraries(MainMorphStructuredGridTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxMorphStructuredGridTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainMorphStructuredGridTest
    ${MimxTesting_SOURCE_DIR}/Data/BoundingBox1.vtk
    ${MimxTesting_SOURCE_DIR}/Data/lumbar_smoothed_04075.stl
    )

  add_executable(MainUnstructuredToStructuredGridTest MainUnstructuredToStructuredGridTest.cxx )
  target_link_libraries(MainUnstructuredToStructuredGridTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxUnstructuredToStructuredGridTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainUnstructuredToStructuredGridTest
    ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_mesh.vtk
    ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk
    )

  add_executable(MainMergePointsTest MainMergePointsTest.cxx )
  target_link_libraries(MainMergePointsTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxMergePointsTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainMergePointsTest
    ${MimxTesting_SOURCE_DIR}/Data/BBoxMerge.vtk
    )

  add_executable(MainRecalculateInteriorNodesTest MainRecalculateInteriorNodesTest.cxx )
  target_link_libraries(MainRecalculateInteriorNodesTest mimxFilter mimxCommon 
                      vtkHybrid vtkRendering vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
  add_test(
    MimxRecalculateInteriorNodesTest
    ${Slicer_EXE} ${WRAPPED_EXE_PREFIX}/MainRecalculateInteriorNodesTest
    ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_mesh.vtk
    ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk
    )




