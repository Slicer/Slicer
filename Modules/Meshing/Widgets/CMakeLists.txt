PROJECT ( MIMXWidgets )

CONFIGURE_FILE(
  ${MIMXWidgets_SOURCE_DIR}/vtkWidgetsConfigure.h.in 
  ${MIMXWidgets_BINARY_DIR}/vtkWidgetsConfigure.h
)

SET ( WIDGETS_SRCS 
    vtkMimxBoundingBoxWidget.cxx
    vtkMimxModPointWidget.cxx
    vtkMimxPolyDataWidget.cxx
    vtkMimxUnstructuredGridExtractWidget.cxx
    vtkMimxUnstructuredGridWidget.cxx
    vtkMimxExtractEdgeWidget.cxx
    vtkMimxExtractFaceWidget.cxx
    vtkMimxExtractCellWidget.cxx
    vtkMimxPlaceLocalAxesWidget.cxx
)

INCLUDE_DIRECTORIES( 
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR}
  ${MIMXWidgets_SOURCE_DIR} ${MIMXWidgets_BINARY_DIR}
  )

ADD_LIBRARY( mimxWidgets ${WIDGETS_SRCS} )

TARGET_LINK_LIBRARIES(mimxWidgets
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  ${KWWidgets_LIBRARIES} 
  mimxCommon
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} mimxWidgets)
ENDIF(BUILD_SHARED_LIBS)

