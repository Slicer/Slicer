PROJECT(OpenCV)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

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
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${OpenIGTLink_INCLUDE_DIRS}
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
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(OpenCV_SRCS 
  vtkOpenCVLogic.cxx
  vtkOpenCVGUI.cxx
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

MESSAGE(STATUS "cv = ${OpenCV_CV_LIBRARY}")


target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
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

#  ${OpenCV_LIBRARIES}
#  ${OpenIGTLink_DIR}
#  ${OpenIGTLinklib}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
