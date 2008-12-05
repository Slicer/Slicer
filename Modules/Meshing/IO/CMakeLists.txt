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
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
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

#ADD_LIBRARY(mimxMeshIO ${MeshIO_TCL_SRCS} ${MeshIO_SRCS} )
ADD_LIBRARY(mimxMeshIO ${MeshIO_SRCS} )

TARGET_LINK_LIBRARIES(mimxMeshIO
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  )





