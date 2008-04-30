project(vtkTeem)
cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Sources

# include a test for endianness (used to be done by default)
include (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(vtkTeem_SRCS
  vtkNRRDReader.cxx
  vtkNRRDWriter.cxx
  vtkDiffusionTensorMathematics.cxx
  vtkDiffusionTensorMathematicsSimple.cxx
  vtkTeemEstimateDiffusionTensor.cxx
  vtkDiffusionTensorGlyph.cxx
  vtkPolyDataTensorToColor.cxx

  vtkBSplineInterpolateImageFunction.cxx
  vtkHyperPointandArray.cxx
  vtkHyperStreamlineDTMRI.cxx
  vtkHyperStreamlineTeem.cxx
  vtkPreciseHyperStreamline.cxx
  vtkPreciseHyperStreamlinePoints.cxx
  vtkSeedTracts.cxx
  vtkTensorImplicitFunctionToFunctionSet.cxx
  vtkTractographyPointAndArray.cxx
  vtkTensorMask.cxx
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

set_source_files_properties(
  vtkHyperPointandArray.cxx
  vtkTractographyPointAndArray.cxx
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(vtkTeem vtkTeem_TCL_SRCS "${vtkTeem_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

add_library(vtkTeem 
  ${vtkTeem_SRCS} 
  ${vtkTeem_TCL_SRCS} 
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} vtkTeem)

if(Slicer3_SOURCE_DIR)
  if(BUILD_SHARED_LIBS)
    install(TARGETS vtkTeem
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
  endif(BUILD_SHARED_LIBS)
endif(Slicer3_SOURCE_DIR)

# after teem 1.9, all the teem libraries are groups in one big libteem.so
if ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  set(TEEM_LIBRARIES teem)
else ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  if ( ${TEEM_VERSION_MINOR} LESS 10 )
    set(TEEM_LIBRARIES air biff nrrd ten ell)
  else ( ${TEEM_VERSION_MINOR} LESS 10 )
    set(TEEM_LIBRARIES teem)
  endif ( ${TEEM_VERSION_MINOR} LESS 10 )
endif ( ${TEEM_VERSION_MAJOR} GREATER 1 )

target_link_libraries(vtkTeem 
  vtkCommon
  vtkFiltering
  vtkImagingTCL
  vtkIOTCL
  vtkGraphicsTCL
  ${TEEM_LIBRARIES}
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

configure_file(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfigure.h.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfigure.h
  )

configure_file(
  ${vtkTeem_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${vtkTeem_BINARY_DIR}/vtkTeemConfig.cmake
  )
