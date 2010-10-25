project(vtkTeem)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Teem REQUIRED)
include(${Teem_USE_FILE})

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
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
  vtk_wrap_tcl3(vtkTeem vtkTeem_TCL_SRCS "${vtkTeem_SRCS}" "")
ENDIF (VTK_WRAP_TCL)

# --------------------------------------------------------------------------
# Build the library

SET(srcs ${vtkTeem_SRCS} )
IF (VTK_WRAP_TCL)
  SET(srcs ${srcs} ${vtkTeem_TCL_SRCS})
ENDIF (VTK_WRAP_TCL)

set(lib_name vtkTeem)
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

SET(VTK_LIBRARIES
  vtkIO
  vtkImaging
  vtkGraphics
  )

SET(libs
  ${Teem_LIBRARIES}
  )

TARGET_LINK_LIBRARIES(${lib_name} ${libs})


FOREACH(c ${VTK_LIBRARIES})
  SET(tcl_wrap)
  IF (VTK_WRAP_TCL)
    SET(tcl_wrap TCL)
  ENDIF()
  TARGET_LINK_LIBRARIES(${lib_name} ${c}${tcl_wrap})
ENDFOREACH(c)

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${vtkTeem_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  
  TARGET_LINK_LIBRARIES(${lib_name}PythonD ${lib_name})
  
  FOREACH(c ${KIT_PYTHON_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name}PythonD ${c}PythonD)
  ENDFOREACH(c)

  TARGET_LINK_LIBRARIES(${lib_name}Python ${lib_name}PythonD)

  ## Python modules on Windows must have the extension ".pyd"
  IF(WIN32 AND NOT CYGWIN)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES SUFFIX ".pyd")
  ENDIF(WIN32 AND NOT CYGWIN)

  # Apply user-defined properties to the library target.
  IF(Slicer_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES
       ${Slicer_LIBRARY_PROPERTIES}
    )
    SET_TARGET_PROPERTIES(${lib_name}PythonD PROPERTIES
       ${Slicer_LIBRARY_PROPERTIES}
    )
  ENDIF(Slicer_LIBRARY_PROPERTIES)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/${PROJECT_NAME} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/${PROJECT_NAME} COMPONENT Development)

ENDIF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkTeemConfig.cmake.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkTeemConfig.cmake
  )

