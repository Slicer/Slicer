PROJECT(Qdec)

CONFIGURE_FILE(
  ${Qdec_SOURCE_DIR}/QdecConfigure.h.in
  ${Qdec_BINARY_DIR}/QdecConfigure.h
)

# --------------------------------------------------------------------------
# Sources


SET(Qdec_SRCS
       QdecProject.cpp
       QdecContrast.cpp
       QdecDataTable.cpp
       QdecFactor.cpp
       QdecGlmDesign.cpp
       QdecGlmFit.cpp
       QdecGlmFitResults.cpp
       QdecSubject.cpp
       QdecUtilities.cpp
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  vtkQdecNode.cxx 
#  ABSTRACT
#  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${Qdec_SOURCE_DIR}
  ${Qdec_BINARY_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
#INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(Qdec Qdec_TCL_SRCS "${Qdec_SRCS}" "")

# Build the library

ADD_LIBRARY(Qdec 
  ${Qdec_SRCS} 
#  ${Qdec_TCL_SRCS}
  )
IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS Qdec
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(Qdec 
  vtkIO
#  vtkCommonTCL
#  vtkIOTCL
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)

SET (tcl_DIR  ${SLICER_LIBRARIES_DIR}/Libs/Qdec)

CONFIGURE_FILE(
  ${Qdec_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  ${tcl_DIR}/vtkFreeSurferReaders.tcl
  COPYONLY
)

# install the tcl file
INSTALL(
  FILES ${tcl_DIR}/vtkFreeSurferReaders.tcl
  DESTINATION lib/Slicer3/Libs/Qdec
)
