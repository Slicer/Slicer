PROJECT(MeshFilter)

#
# Sources
#
IF(VTK_MAJOR_VERSION LESS 5)
  SET(AddVtk_SRCS vtkTable.cxx vtkTableReader.cxx)
ELSE(VTK_MAJOR_VERSION LESS 5)
  IF(VTK_MAJOR_VERSION EQUAL 5)
    IF(VTK_MINOR_VERSION LESS 1)
      SET(AddVtk_SRCS vtkTable.cxx vtkTableReader.cxx)
    ENDIF(VTK_MINOR_VERSION LESS 1)
  ENDIF(VTK_MAJOR_VERSION EQUAL 5)
ENDIF(VTK_MAJOR_VERSION LESS 5)

SET(MeshFilter_SRCS 
  vtkRebinMaterialProperty.cxx
  ${AddVtk_SRCS}
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
# Wrapping - Temp Remove 
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MeshFilter MeshFilter_TCL_SRCS "${MeshFilter_SRCS}" "")


# Build the library
#
ADD_LIBRARY(MimxMeshFilter 
  ${MeshFilter_SRCS} 
#  ${MeshFilter_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} MimxMeshFilter)
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
  ${MeshFilter_SOURCE_DIR}/vtkMeshFilterConfigure.h.in 
  ${MeshFilter_BINARY_DIR}/vtkMeshFilterConfigure.h
  )

INSTALL_TARGETS(/lib/MimxMesh MimxMeshFilter)
INSTALL_FILES(/include/MimxMesh/Filter "(\\.h|\\.txx)$")
