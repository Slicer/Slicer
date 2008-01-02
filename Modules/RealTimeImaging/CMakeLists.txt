PROJECT(RealTimeImaging)

#Check if NaviTrack is switch on, else don't compile that module!

SET(USE_NAVITRACK 0)
IF(USE_NAVITRACK)

#
# Sources
#
SET(RealTimeImaging_SRCS 
  # module
  vtkRealTimeImagingLogic.cxx
  vtkRealTimeImagingGUI.cxx
  #Image.cxx
  )

#
# Include dirs
#
INCLUDE_DIRECTORIES(
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${RealTimeImaging_SOURCE_DIR}
  ${RealTimeImaging_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${NAVITRACK_INC}
  )

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(RealTimeImaging RealTimeImaging_TCL_SRCS "${RealTimeImaging_SRCS}" "")
#Simond
#INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
#KWWidgets_WRAP_TCL(RealTimeImaging RealTimeImaging_TCL_SRCS "${RealTimeImaging_SRCS}" "")

#
# Build the library
#
ADD_LIBRARY(RealTimeImaging
  ${RealTimeImaging_SRCS} 
  ${RealTimeImaging_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} RealTimeImaging)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(RealTimeImaging
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
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
  vtkHybridTCL
  ${NAVITRACK_LIB}
  )

#
# Configure include file
#
CONFIGURE_FILE(
  ${RealTimeImaging_SOURCE_DIR}/vtkRealTimeImagingConfigure.h.in 
  ${RealTimeImaging_BINARY_DIR}/vtkRealTimeImagingConfigure.h
  )
 
ELSE(USE_NAVITRACK)
  # MESSAGE("You have to turn NAVITRACK 'ON' to compile RealTimeImageing")
ENDIF(USE_NAVITRACK)
