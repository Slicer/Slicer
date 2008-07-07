project(vtkTeem)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(TEEM REQUIRED)
include(${TEEM_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif(NOT DEFINED BUILD_SHARED_LIBS)
 
# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkTeemConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkTeemConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkTeemConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

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
  vtkTensorRotate.cxx
  vtkImageLabelCombine.cxx
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
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(vtkTeem vtkTeem_TCL_SRCS "${vtkTeem_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

set(lib_name vtkTeem)
add_library(${lib_name} 
  ${vtkTeem_SRCS} 
  ${vtkTeem_TCL_SRCS} 
  )

# after teem 1.9, all the teem libraries are groups in one big libteem.so
if ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  set(TEEM_LIBRARIES teem)
  MESSAGE(FATAL "GREATER 1 ${TEEM_LIBRARIES}")
else ( ${TEEM_VERSION_MAJOR} GREATER 1 )
  if ( ${TEEM_VERSION_MINOR} LESS 10 )
    set(TEEM_LIBRARIES air biff nrrd ten ell)
  MESSAGE(FATAL "LESS 10 ${TEEM_LIBRARIES}")
  else ( ${TEEM_VERSION_MINOR} LESS 10 )
    set(TEEM_LIBRARIES teem)
  MESSAGE(FATAL "NOT LESS 10 ${TEEM_LIBRARIES}")
  endif ( ${TEEM_VERSION_MINOR} LESS 10 )
endif ( ${TEEM_VERSION_MAJOR} GREATER 1 )

target_link_libraries(${lib_name} 
  vtkCommon
  vtkFiltering
  vtkImagingTCL
  vtkIOTCL
  vtkGraphicsTCL
  ${TEEM_LIBRARIES}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkTeemConfig.cmake
  )

