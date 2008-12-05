PROJECT( MIMXFilter )


# Include dirs
#
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR} 
  ${mimxCommon_SOURCE_DIR} ${mimxCommon_BINARY_DIR} 
  ${MIMXCommon_SOURCE_DIR}../Adapter ${MIMXCommon_BINARY_DIR}../Adapter 
  ${MIMXAdapter_SOURCE_DIR} ${MIMXAdapter_BINARY_DIR} 
  ${VTK_SOURCE_DIR}/Utilities/verdict/include ${VTK_KITS_DIR}/verdict 
)


configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxFilterConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxFilterConfigure.h
)
  

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${MIMXCommon_BINARY_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MIMXFilter MIMXFilter_TCL_SRCS "${MIMXFilter_SRCS}" "")


SET ( FILTER_SRCS 
    vtkMimxEdgeToStructuredGrid.cxx            
    vtkMimxPlanarTransfiniteInterpolation.cxx  
    vtkMimxSubdivideCurve.cxx
    vtkMimxBoundingBoxSurfaceMesh.cxx    
    vtkMimxEditUnstructuredHexahedronGrid.cxx  
    vtkMimxSmoothUnstructuredGridFilter.cxx
    vtkMimxClipImageFromBox.cxx          
    vtkMimxGenerateHexahedronMesh.cxx          
    vtkMimxSolidTransfiniteInterpolation.cxx
    vtkMimxCylindricalMesh.cxx
    vtkMimxEdgeToPlanarStructuredGrid.cxx
    vtkMimxGeometryFilter.cxx
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
    vtkMimxApplyImageBasedMaterialProperties.cxx
    vtkMimxExtractElementNumbersOfaFaceFilter.cxx
    vtkMimxSubdivideBoundingBox.cxx
    vtkMimxRecalculateInteriorNodesTPS.cxx
    vtkMimxBoundingBoxFromUnstructuredGrid.cxx
    vtkMimxMapOriginalCellAndPointIds.cxx
    vtkMimxEquivalancePoints.cxx
    vtkMeshQualityExtended.cxx
    vtkMimxMergePoints.cxx
    vtkFaceHedgeHog.cxx
    vtkMimxExtractSurface.cxx
)


ADD_LIBRARY( mimxFilter ${FILTER_SRCS} )

TARGET_LINK_LIBRARIES(mimxFilter
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkRendering
  ITKCommon
  ITKBasicFilters
  ITKIO
  ITKAlgorithms
  mimxCommon
  ${KWWidgets_LIBRARIES}
  )


