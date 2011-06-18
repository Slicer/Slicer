PROJECT (MIMXWidgets)

SET (LIB_NAME "mimxWidgets")

SET ( LIB_SRCS 
    vtkMimxBoundingBoxWidget.cxx
    #vtkMimxBWidgetPlusIPWidget.cxx
    vtkMimxCreateBuildingBlockFromPickWidget.cxx
    vtkMimxCreateElementSetWidgetFEMesh.cxx
    vtkMimxModPointWidget.cxx
    vtkMimxPolyDataWidget.cxx
    vtkMimxUnstructuredGridExtractWidget.cxx
    vtkMimxUnstructuredGridWidget.cxx
    vtkMimxExtractEdgeWidget.cxx
    vtkMimxExtractFaceWidget.cxx
    vtkMimxExtractCellWidget.cxx
    vtkMimxPlaceLocalAxesWidget.cxx
    vtkMimxTraceContourWidget.cxx
    vtkMimxExtractMultipleFaceWidget.cxx
    vtkMimxSelectCellsWidget.cxx
    vtkMimxSelectPointsWidget.cxx
    vtkMimxSelectCellsWidgetFEMesh.cxx
    #vtkMimxColorCodeMeshSeedWidget.cxx
    #
    vtkMimxViewMergePointsWidget.cxx
)

SET_SOURCE_FILES_PROPERTIES(
    vtkMimxBoundingBoxWidget.cxx
    #vtkMimxBWidgetPlusIPWidget.cxx
    vtkMimxCreateElementSetWidgetFEMesh.cxx
    vtkMimxModPointWidget.cxx
    vtkMimxPolyDataWidget.cxx
    vtkMimxUnstructuredGridExtractWidget.cxx
    vtkMimxUnstructuredGridWidget.cxx
    vtkMimxExtractEdgeWidget.cxx
    vtkMimxExtractFaceWidget.cxx
    vtkMimxExtractCellWidget.cxx
    vtkMimxPlaceLocalAxesWidget.cxx
    vtkMimxTraceContourWidget.cxx
    vtkMimxExtractMultipleFaceWidget.cxx
    vtkMimxSelectCellsWidget.cxx
    vtkMimxSelectPointsWidget.cxx
    vtkMimxSelectCellsWidgetFEMesh.cxx
    #vtkMimxColorCodeMeshSeedWidget.cxx
  WRAP_EXCLUDE
  )


#INCLUDE_DIRECTORIES( ${IAFEMMESH_SOURCE_DIR}/Code/Common ${IAFEMMESH_BINARY_DIR}/Code/Common )
#INCLUDE_DIRECTORIES( ${IAFEMMESH_SOURCE_DIR}/Code/Widgets ${IAFEMMESH_BINARY_DIR}/Code/Widgets )
#INCLUDE_DIRECTORIES( ${IAFEMMESH_SOURCE_DIR}/Code/Filter ${IAFEMMESH_BINARY_DIR}/Code/Filter )

# Include dirs
#
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_INCLUDE_DIRS}
  ${MRML_INCLUDE_DIRS}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR} 
  ${MIMXAdapter_SOURCE_DIR} ${MIMXAdapter_BINARY_DIR} 
  ${MIMXWidgets_SOURCE_DIR} ${MIMXWidgets_BINARY_DIR} 
  ${MIMXFilter_SOURCE_DIR} ${MIMXFilter_BINARY_DIR} 
  ${MIMXIO_SOURCE_DIR} ${MIMXIO_BINARY_DIR} 
  ${MIMXMesh_SOURCE_DIR} ${MIMXMesh_BINARY_DIR} 
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxWidgetsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxWidgetsConfigure.h
  )
  
 
ADD_LIBRARY( mimxWidgets ${LIB_TCL_SRCS} ${LIB_SRCS} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library. The library is built in static mode for convenience. 
# Check the 'Callbacks' example for more information about building it in
# shared mode, i.e. without the STATIC keyword (Win32 compilers requires
# an additional header file to setup DLL export symbols correctly).

TARGET_LINK_LIBRARIES(${LIB_NAME}
  ModuleDescriptionParser
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  ${KWWidgets_LIBRARIES} 
  mimxWidgets
  mimxFilter
  mimxCommon
  )

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)
  
install(TARGETS ${LIB_NAME}
  RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development
  )

