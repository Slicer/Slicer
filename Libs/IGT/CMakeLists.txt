project(IGT)

# --------------------------------------------------------------------------
# Special build configs (to handle linking on windows)
configure_file(
  ${IGT_SOURCE_DIR}/vtkIGTConfigure.h.in 
  ${IGT_BINARY_DIR}/vtkIGTConfigure.h
  )

# --------------------------------------------------------------------------
# Settings for options
set(IGT_SRCS
  vtkIGTDataManager.cxx
  vtkIGTDataStream.cxx
  vtkIGTMatrixState.cxx
  vtkIGTPat2ImgRegistration.cxx
  )
set(TRACKING_INTERFACE_LIBRARY "")

#--- NH took out the following section in an effot to remove Navitrak
#--- 10/16/07

#IF(USE_NAVITRACK)
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
#ENDIF(USE_NAVITRACK)

if(USE_IGSTK)
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
endif(USE_IGSTK)

# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${IGT_BINARY_DIR}
  ${IGT_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

# --------------------------------------------------------------------------
# Wrapping
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(IGT IGT_TCL_SRCS "${IGT_SRCS}" "")

# Build the library
add_library(IGT 
  ${IGT_SRCS} ${IGT_TCL_SRCS}
  )

GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} IGT)

if(BUILD_SHARED_LIBS)
  install(TARGETS IGT
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(IGT
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

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)
