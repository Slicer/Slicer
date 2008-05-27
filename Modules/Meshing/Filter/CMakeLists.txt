project(MIMXFilter)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MimxMeshing_SOURCE_DIR}/Common 
  ${MimxMeshing_BINARY_DIR}/Common 
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkFilterConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkFilterConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB templates "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
install(FILES 
  ${headers} 
  ${templates} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkFilterConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(MIMXFilter_SRCS 
  vtkMimxEdgeToStructuredGrid.cxx            
  vtkMimxPlanarTransfiniteInterpolation.cxx  
  vtkMimxSubdivideCurve.cxx
  #vtkMimxBoundingBoxSurfaceMesh.cxx    
  vtkMimxEditUnstructuredHexahedronGrid.cxx  
  vtkMimxSmoothUnstructuredGridFilter.cxx
  #vtkMimxClipImageFromBox.cxx          
  vtkMimxGenerateHexahedronMesh.cxx          
  vtkMimxSolidTransfiniteInterpolation.cxx
  vtkMimxCylindricalMesh.cxx
  vtkMimxEdgeToPlanarStructuredGrid.cxx
  vtkMimxStructuredPlanarToStructuredSolidGrid.cxx
  vtkMimxSolidTrilinearInterpolation.cxx
  vtkMimxAddUnstructuredHexahedronGridCell.cxx
  vtkMimxDeleteUnstructuredHexahedronGridCell.cxx
  vtkMimxMirrorUnstructuredHexahedronGridCell.cxx
  vtkMimxSplitUnstructuredHexahedronGridCell.cxx
  vtkMimxPatchSurfaceMesh.cxx
  vtkMimxUnstructuredGridFromFourPoints.cxx
  vtkMimxPolyDataSingleSourceShortestPath.cxx
  vtkMimxFourPointsToStructuredPlanarMesh.cxx
  vtkMimxBoundingBoxToStructuredGrids.cxx
  vtkMimxComputeNormalsFromPolydataFilter.cxx
  vtkMimxPlanarEllipticalInterpolation.cxx
  vtkMimxSolidEllipticalInterpolation.cxx
  vtkMimxPlanarTrilinearInterpolation.cxx
  vtkMimxSelectSurface.cxx
  vtkMimxExtrudePolyData.cxx
  vtkMimxAttachBoundingBoxMesh.cxx
  vtkMimxSetStructuredGridEdge.cxx
  vtkMimxExtractStructuredGridEdge.cxx
  vtkMimxMorphStructuredGrid.cxx
  vtkMimxSetStructuredGridFace.cxx
  vtkMimxExtractStructuredGridFace.cxx
  vtkMimxConstrainedSmoothPolyDataFilter.cxx
  vtkMimxRecalculateInteriorNodes.cxx
  vtkMimxUnstructuredGridFromBoundingBox.cxx
  vtkMimxButterflyMeshFromBounds.cxx
  vtkMimxGenerateIntervertebralDisc.cxx
  vtkMimxUnstructuredToStructuredGrid.cxx
  vtkMimxRebinMaterialProperty.cxx
  vtkMimxMeshSeed.cxx
  vtkMimxBoundingBoxFromBounds.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name mimxFilter)
add_library(${lib_name} 
  ${MIMXFilter_SRCS}
)
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})
