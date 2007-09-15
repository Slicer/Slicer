PROJECT( MIMXFilter )

CONFIGURE_FILE(
  ${MIMXFilter_SOURCE_DIR}/vtkFilterConfigure.h.in 
  ${MIMXFilter_BINARY_DIR}/vtkFilterConfigure.h
)

SET ( FILTER_SRCS 
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

INCLUDE_DIRECTORIES( 
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR}
  ${MIMXFilter_SOURCE_DIR} ${MIMXFilter_BINARY_DIR}
  )

ADD_LIBRARY( mimxFilter ${FILTER_SRCS} )

TARGET_LINK_LIBRARIES(mimxFilter
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkRendering
  ${KWWidgets_LIBRARIES} 
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} mimxFilter)
ENDIF(BUILD_SHARED_LIBS)

