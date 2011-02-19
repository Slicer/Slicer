PROJECT(MIMXAdapter)

#CONFIGURE_FILE(
#  ${MIMXAdapter_SOURCE_DIR}/vtkMimxAdapterConfigure.h.in
#  ${MIMXAdapter_BINARY_DIR}/vtkMimxAdapterConfigure.h
#  )

SET (MIMXAdapter_SRCS 
    vtkUnstructuredGridToitkMesh.cxx 
    itkMeshTovtkPolyData.cxx  
    vtkPolyDataToitkMesh.cxx
    ) 

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
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  ${MIMXCommon_SOURCE_DIR} ${MIMXCommon_BINARY_DIR} 
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxAdapterConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxAdapterConfigure.h
  )
  
#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${MIMXAdapter_BINARY_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(mimxAdapter MimxAdapter_TCL_SRCS "${MIMXAdapter_SRCS}" "")

SET(LIB_NAME "mimxAdapter")

ADD_LIBRARY(${LIB_NAME} ${MIMXAdapter_SRCS} )

TARGET_LINK_LIBRARIES(${LIB_NAME} 
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  ${ITK_LIBRARIES}
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
  
