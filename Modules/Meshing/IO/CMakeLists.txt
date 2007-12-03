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
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${MeshIO_BINARY_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(MeshIO MeshIO_TCL_SRCS "${MeshIO_SRCS}" "")


# Build the library
#
ADD_LIBRARY(MimxMeshIO
  ${MeshIO_SRCS} 
#  ${MeshIO_TCL_SRCS}
  )

TARGET_LINK_LIBRARIES( MimxMeshIO 
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  vtkWidgets
  vtkRendering
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS MimxMeshIO
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)


#
# Configure include file
#
CONFIGURE_FILE(
  ${MeshIO_SOURCE_DIR}/vtkMeshIOConfigure.h.in 
  ${MeshIO_BINARY_DIR}/vtkMeshIOConfigure.h
  )

FILE(GLOB __files1 "${MeshIO_SOURCE_DIR}/*.h")
# FILE(GLOB __files2 "${MeshIO_SOURCE_DIR}/*.txx")
# INSTALL(FILES ${__files1} {$__files2} DESTINATION include/MimxMesh/IO COMPONENT Development)
INSTALL(FILES ${__files1} DESTINATION include/MimxMesh/IO COMPONENT Development)
