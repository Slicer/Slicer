project(MimxTesting)

include_regular_expression("^.*$")

include_directories(
  ${MimxMeshing_SOURCE_DIR}/Filter
  ${MimxMeshing_BINARY_DIR}/Filter
  ${MimxMeshing_SOURCE_DIR}/Mesh
  ${MimxMeshing_BINARY_DIR}/Common
  ${MimxMeshing_SOURCE_DIR}/Common
  ${MimxMeshing_SOURCE_DIR}/IO
  ${MimxMeshing_BINARY_DIR}/IO
  )

set(MIMX_LIBRARIES mimxCommon mimxFilter)

if(ITK_BINARY_DIR AND TCL_TCLSH)
  add_test(PrintSelf-BasicFilters ${TCL_TCLSH}
    ${ITK_SOURCE_DIR}/Utilities/Dart/PrintSelfCheck.tcl
    ${ITK_SOURCE_DIR}/Code/BasicFilters)
endif()

macro(MIMX_ADD_TEST TESTNAME SOURCE_FILE)
  get_filename_component(mimx_test_executable ${SOURCE_FILE} NAME_WE)
  add_executable(${TESTNAME} ${SOURCE_FILE})
  target_link_libraries(${TESTNAME} ${MIMX_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES})
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${TESTNAME}> ${ARGN})
endmacro()

# MIMX_ADD_TEST(vtkRebinMaterialPropertyTest vtkRebinMaterialPropertyTest.cxx)

MIMX_ADD_TEST(MimxitkImageToVtkUnstructuredGridTest itkImageToVtkUnstructuredGridTest.cxx)

MIMX_ADD_TEST(MimxAddUnstructuredHexahedronGridCellTest MainAddUnstructuredHexahedronGridCellTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk)

MIMX_ADD_TEST(MimxDeleteUnstructuredHexahedronGridCellTest MainDeleteUnstructuredHexahedronGridCellTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBox6.vtk)

MIMX_ADD_TEST(MimxMirrorUnstructuredHexahedronGridCellTest MainMirrorUnstructuredHexahedronGridCellTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk)

MIMX_ADD_TEST(MimxUnstructuredHexahedronGridCellTest MainSplitUnstructuredHexahedronGridCellTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk)

############## Re-Enable when Meshing Integratation is completed
#MIMX_ADD_TEST(MimxApplyImageBasedMaterialPropertiesTest MainApplyImageBasedMaterialPropertiesTest.cxx
#              ${MimxTesting_SOURCE_DIR}/Data/CA05042124RFinal.hdr
#              ${MimxTesting_SOURCE_DIR}/Data/CAfemesh.vtk)
##############

MIMX_ADD_TEST(MimxBoundingBoxFromUnstructuredGridTest MainBoundingBoxFromUnstructuredGridTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/dividedbb.vtk
              ${MimxTesting_SOURCE_DIR}/Data/dividedmorphsmooth.vtk)

MIMX_ADD_TEST(MimxEquivalancePointsTest MainEquivalancePointsTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/Clubfoot_Link_38_Mesh.vtk)

MIMX_ADD_TEST(MimxExtractSurfaceTest MainExtractSurfaceTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBox2.vtk)

MIMX_ADD_TEST(MimxBoundingBoxFromBoundsTest MainBoundingBoxFromBoundsTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/bbox2_modified.vtk)

MIMX_ADD_TEST(MimxExtractStructuredGridEdgeTest MainExtractStructuredGridEdgeTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/sgrid.vtk)

MIMX_ADD_TEST(MimxExtractStructuredGridFaceTest MainExtractStructuredGridFaceTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/sgrid.vtk)

MIMX_ADD_TEST(MimxGeometryFilterTest MainGeometryFilterTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/Vert_Bod.vtk)

MIMX_ADD_TEST(MimxBoundingBoxToStructuredGridsTest MainBoundingBoxToStructuredGridsTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk)

MIMX_ADD_TEST(MimxMorphStructuredGridTest MainMorphStructuredGridTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BoundingBox1.vtk
              ${MimxTesting_SOURCE_DIR}/Data/lumbar_smoothed_04075.stl)

MIMX_ADD_TEST(MimxUnstructuredToStructuredGridTest MainUnstructuredToStructuredGridTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_mesh.vtk
              ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk)

MIMX_ADD_TEST(MimxMergePointsTest MainMergePointsTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/BBoxMerge.vtk)

MIMX_ADD_TEST(MimxRecalculateInteriorNodesTest MainRecalculateInteriorNodesTest.cxx
              ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_mesh.vtk
              ${MimxTesting_SOURCE_DIR}/Data/1P-MD05010306R_2Box_seeds.vtk)


