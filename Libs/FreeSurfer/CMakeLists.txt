project(FreeSurfer)

configure_file(
  ${FreeSurfer_SOURCE_DIR}/FreeSurferConfigure.h.in
  ${FreeSurfer_BINARY_DIR}/FreeSurferConfigure.h
  )

# --------------------------------------------------------------------------
# Sources

set(FreeSurfer_SRCS
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

set(include_dirs
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(FreeSurfer FreeSurfer_TCL_SRCS "${FreeSurfer_SRCS}" "")

# Build the library

add_library(FreeSurfer 
  ${FreeSurfer_SRCS} ${FreeSurfer_TCL_SRCS}
  )

if(BUILD_SHARED_LIBS)
  install(TARGETS FreeSurfer
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(FreeSurfer 
  vtkIO
  vtkCommonTCL
  vtkIOTCL
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} FreeSurfer)

# Copy the colour files to the build directory
set(COLORFILES
  FreeSurferColorLUT.txt
  Simple_surface_labels2002.txt    
  )

set(Color_DIR  ${SLICER_LIBRARIES_DIR}/Libs/FreeSurfer)

foreach(colorfile ${COLORFILES})
  configure_file(
    ${FreeSurfer_SOURCE_DIR}/${colorfile}
    ${Color_DIR}/${colorfile}
    COPYONLY
    )
  install(
    FILES ${Color_DIR}/${colorfile}
    DESTINATION lib/Slicer3/Libs/FreeSurfer
    )
endforeach(colorfile)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

# IF(BUILD_TESTING)
#  SUBDIRS(Testing)
# ENDIF(BUILD_TESTING)
