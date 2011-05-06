project(vtkTeem)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()
 
# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkTeemConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in 
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  SET(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
ENDIF()
IF(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.(h|txx)")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
ENDIF()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
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
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})

SET(srcs ${vtkTeem_SRCS})
add_library(${lib_name} ${srcs})

# after teem 1.9, all the teem libraries are groups in one big libteem.so
if ( ${Teem_VERSION_MAJOR} GREATER 1 )
  set(Teem_LIBRARIES teem)
else ( ${Teem_VERSION_MAJOR} GREATER 1 )
  if ( ${Teem_VERSION_MINOR} LESS 10 )
    set(Teem_LIBRARIES air biff nrrd ten ell)
  else ( ${Teem_VERSION_MINOR} LESS 10 )
    set(Teem_LIBRARIES teem)
  endif ( ${Teem_VERSION_MINOR} LESS 10 )
endif ( ${Teem_VERSION_MAJOR} GREATER 1 )

SET(libs
  ${Teem_LIBRARIES}
  ${VTK_LIBRARIES}
  )
TARGET_LINK_LIBRARIES(${lib_name} ${libs})

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer_LIBRARY_PROPERTIES})
ENDIF()

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  SET(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
ENDIF()
IF(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  SET(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
ENDIF()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
IF(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${vtkTeem_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
ENDIF()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif()

# --------------------------------------------------------------------------
# Configure support file
# --------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkTeemConfig.cmake
  )

