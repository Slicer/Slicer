PROJECT (MIMXMeshIO)

CONFIGURE_FILE(
  ${MIMXMeshIO_SOURCE_DIR}/vtkMeshIOConfigure.h.in
  ${MIMXMeshIO_BINARY_DIR}/vtkMeshIOConfigure.h
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
  ${vtkITK_INCLUDE_DIRS}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMimxIOConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMimxIOConfigure.h
  )
  
SET (MeshIO_SRCS 
    vtkMimxAbaqusFileWriter.cxx
    vtkMimxIntervertebralDiscAbaqusWriter.cxx
    vtkPluginFilterWatcher.cxx
    ModuleProcessInformation.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkMimxAbaqusFileWriter.cxx
  WRAP_EXCLUDE
)
  
#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MIMXMeshIO MeshIO_TCL_SRCS "${MeshIO_SRCS}" "")
SET(LIB_NAME "mimxMeshIO")

#ADD_LIBRARY(mimxMeshIO ${MeshIO_TCL_SRCS} ${MeshIO_SRCS} )
ADD_LIBRARY(${LIB_NAME} ${MeshIO_SRCS} )

TARGET_LINK_LIBRARIES(${LIB_NAME}
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
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
  



