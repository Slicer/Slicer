project( MIMXFilter )


# Include dirs
#
include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR}
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MIMXCommon_BINARY_DIR})
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MIMXFilter MIMXFilter_TCL_SRCS "${MIMXFilter_SRCS}" "")

set(LIB_NAME "mimxFilter")

set( FILTER_SRCS
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


add_library( ${LIB_NAME} ${FILTER_SRCS} )

target_link_libraries(${LIB_NAME}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  mimxCommon
  )

# Apply user-defined properties to the library target.
if(Slicer_LIBRARY_PROPERTIES)
  set_target_properties(${LIB_NAME} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
endif(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

