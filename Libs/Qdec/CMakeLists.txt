project(Qdec)

configure_file(
  ${Qdec_SOURCE_DIR}/QdecConfigure.h.in
  ${Qdec_BINARY_DIR}/QdecConfigure.h
  )

# --------------------------------------------------------------------------
# Sources

set(Qdec_SRCS
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

set(include_dirs
  ${Qdec_SOURCE_DIR}
  ${Qdec_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Wrapping

#INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(Qdec Qdec_TCL_SRCS "${Qdec_SRCS}" "")

# Build the library

add_library(Qdec 
  ${Qdec_SRCS} 
  #  ${Qdec_TCL_SRCS}
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} Qdec)

if(BUILD_SHARED_LIBS)
  install(TARGETS Qdec
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(Qdec 
  vtkIO
  #  vtkCommonTCL
  #  vtkIOTCL
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)

set(tcl_DIR  ${SLICER_LIBRARIES_DIR}/Libs/Qdec)

configure_file(
  ${Qdec_SOURCE_DIR}/vtkFreeSurferReaders.tcl
  ${tcl_DIR}/vtkFreeSurferReaders.tcl
  COPYONLY
  )

# install the tcl file
install(
  FILES ${tcl_DIR}/vtkFreeSurferReaders.tcl
  DESTINATION lib/Slicer3/Libs/Qdec
  )
