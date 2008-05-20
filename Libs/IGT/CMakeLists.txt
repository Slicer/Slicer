project(IGT)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkIGTConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkIGTConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkIGTConfigure.h"
  DESTINATION include/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(IGT_SRCS
  vtkIGTDataManager.cxx
  vtkIGTDataStream.cxx
  vtkIGTMatrixState.cxx
  vtkIGTPat2ImgRegistration.cxx
  )
set(TRACKING_INTERFACE_LIBRARY "")

#--- NH took out the following section in an effot to remove Navitrak
#--- 10/16/07

#IF(Slicer3_USE_NAVITRACK)
#  SET(IGT_SRCS
#    vtkIGTDataManager.cxx
#    vtkIGTDataStream.cxx
#    vtkIGTMatrixState.cxx
#    vtkIGTPat2ImgRegistration.cxx
#    vtkIGTOpenTrackerStream.cxx
#  )
#  SET(TRACKING_INTERFACE_INCLUDE_DIR
#    ${NAVITRACK_INC}
#  )
#  SET(TRACKING_INTERFACE_LIB  
#    ${NAVITRACK_LIB}
#  )
#ENDIF(Slicer3_USE_NAVITRACK)

if(Slicer3_USE_IGSTK)
  set(IGT_SRCS
    vtkIGTDataManager.cxx
    vtkIGTDataStream.cxx
    vtkIGTMatrixState.cxx
    vtkIGTPat2ImgRegistration.cxx
    vtkIGTIGSTKStream.cxx
    )
  set(TRACKING_INTERFACE_INCLUDE_DIR
    ${IGSTK_INCLUDE_DIRS}
    )
  set(TRACKING_INTERFACE_LIB  
    ${IGSTK_LIBRARIES}
    )
endif(Slicer3_USE_IGSTK)

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(IGT IGT_TCL_SRCS "${IGT_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name IGT)
add_library(${lib_name} 
  ${IGT_SRCS} 
  ${IGT_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  MRML
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkHybridTCL
  vtkIOTCL
  vtkGraphicsTCL
  vtkRenderingTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES}
  ${TRACKING_INTERFACE_LIB}
  )

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${lib_name})

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
