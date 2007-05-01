PROJECT(MeshIO)

#
# Sources
#
SET(MeshIO_SRCS 
  vtkAbaqusFileWriter.cxx
)

# Include dirs
#
INCLUDE_DIRECTORIES(
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

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MeshIO MeshIO_TCL_SRCS "${MeshIO_SRCS}" "")


# Build the library
#
ADD_LIBRARY(MimxMeshIO 
  ${MeshIO_SRCS} 
#  ${MeshIO_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} MimxMeshIO)
ENDIF(BUILD_SHARED_LIBS)

#
# Testing
#
IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

#
# Configure include file
#
CONFIGURE_FILE(
  ${MeshIO_SOURCE_DIR}/vtkMeshIOConfigure.h.in 
  ${MeshIO_BINARY_DIR}/vtkMeshIOConfigure.h
  )

INSTALL_TARGETS(/lib/MimxMesh MimxMeshIO)
INSTALL_FILES(/include/MimxMesh/IO "(\\.h|\\.txx)$")
