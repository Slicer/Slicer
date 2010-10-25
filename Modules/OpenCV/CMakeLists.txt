PROJECT(OpenCV)

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

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# OpenCV stuff

# Chinzei; Next 1 line is to include here to find FindXX.cmake files.
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


# Chinzei; This does eerything you want!
FIND_PACKAGE(OpenCV REQUIRED)

#SET(CVlib  cv cvaux cxcore ml highgui)
#SET(CVlibPath   /Users/ayamada/opencv-1.0.0/cv/.libs
#        /Users/ayamada/opencv-1.0.0/cvaux/.libs
#        /Users/ayamada/opencv-1.0.0/cxcore/.libs
#        /Users/ayamada/opencv-1.0.0/ml/.libs
#        /Users/ayamada/opencv-1.0.0/otherlibs/highgui/.libs)
#SET(CVincludePath  /Users/ayamada/opencv-1.0.0/cv/include
#           /Users/ayamada/opencv-1.0.0/cvaux/include
#           /Users/ayamada/opencv-1.0.0/cxcore/include
#           /Users/ayamada/opencv-1.0.0/ml/include
#           /Users/ayamada/opencv-1.0.0/otherlibs/highgui)
#
#LINK_DIRECTORIES(${CVlibPath})

# --------------------------------------------------------------------------

# --------------------------------------------------------------------------
# OpenIGTLink stuff

FIND_PACKAGE(OpenIGTLink REQUIRED)

##SET(OpenIGTLinklib  cv cvaux cxcore ml highgui)
#SET(OpenIGTLinklib OpenIGTLink)
##SET(CVlibPath   /Users/ayamada/opencv-1.0.0/cv/.libs
##        /Users/ayamada/opencv-1.0.0/cvaux/.libs
##        /Users/ayamada/opencv-1.0.0/cxcore/.libs
##        /Users/ayamada/opencv-1.0.0/ml/.libs
##        /Users/ayamada/opencv-1.0.0/otherlibs/highgui/.libs)
#SET(OpenIGTLinklibPath   /Users/ayamada/videoopenigtlink/OpenIGTLinkVideoServer/lib)
##SET(OpenIGTLinkincludePath  /Users/ayamada/OpenIGTLink/OpenIGTLink/Source)
#SET(OpenIGTLinkincludePath  /Users/ayamada/videoopenigtlink/OpenIGTLinkVideoServer/include/OpenIGTLink)
##LINK_DIRECTORIES(${CVlibPath})
#
#LINK_DIRECTORIES(${OpenIGTLinklibPath})
# --------------------------------------------------------------------------

# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${OpenIGTLink_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenCVConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkOpenCVConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkOpenCVConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(OpenCV_SRCS 
  vtkOpenCVGUI.cxx
  vtkOpenCVLogic.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(OpenCV OpenCV_TCL_SRCS "${OpenCV_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(OpenCV_SRCS OpenCV.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name OpenCV)
add_library(${lib_name}
  ${OpenCV_SRCS} 
  ${OpenCV_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${OpenCV_CV_LIBRARY}
  ${OpenCV_CVAUX_LIBRARY}
#  ${OpenCV_CVCAM_LIBRARY}
  ${OpenCV_CVHAARTRAINING_LIBRARY}
  ${OpenCV_CXCORE_LIBRARY}
#  ${OpenCV_CXTS_LIBRARY} 
  ${OpenCV_HIGHGUI_LIBRARY}
  ${OpenCV_ML_LIBRARY}
#  ${OpenCV_TRS_LIBRARY}
  ${OpenIGTLinklib}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
