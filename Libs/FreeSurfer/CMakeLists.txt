PROJECT(FreeSurfer)

CONFIGURE_FILE(
  ${FreeSurfer_SOURCE_DIR}/FreeSurferConfigure.h.in
  ${FreeSurfer_BINARY_DIR}/FreeSurferConfigure.h
)

# --------------------------------------------------------------------------
# Sources


SET(FreeSurfer_SRCS
        vtkFSIO.cxx
        vtkFSSurfaceReader.cxx
        vtkFSSurfaceAnnotationReader.cxx
        vtkFSSurfaceScalarReader.cxx
        vtkFSSurfaceWFileReader.cxx
        vtkFSLookupTable.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  vtkFreeSurferNode.cxx 
#  ABSTRACT
#  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(FreeSurfer FreeSurfer_TCL_SRCS "${FreeSurfer_SRCS}" "")

# Build the library

ADD_LIBRARY(FreeSurfer 
  ${FreeSurfer_SRCS} ${FreeSurfer_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS FreeSurfer
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(FreeSurfer 
  vtkIO
  vtkCommonTCL
  vtkIOTCL
  )

# Copy the colour files to the build directory
SET (COLORFILES
    FreeSurferColorLUT.txt
    Simple_surface_labels2002.txt    
)

SET (Color_DIR  ${SLICER_LIBRARIES_DIR}/Libs/FreeSurfer)

FOREACH ( colorfile ${COLORFILES})
  CONFIGURE_FILE(
    ${FreeSurfer_SOURCE_DIR}/${colorfile}
    ${Color_DIR}/${colorfile}
    COPYONLY
)
  INSTALL(
    FILES ${Color_DIR}/${colorfile}
  DESTINATION lib/Slicer3/Libs/FreeSurfer
)
ENDFOREACH(colorfile)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)
