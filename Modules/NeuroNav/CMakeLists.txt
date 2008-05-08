PROJECT(NeuroNav)

IF(NOT USE_IGSTK)
 ADD_DEFINITIONS(-DIGSTK_OFF)
ENDIF(NOT USE_IGSTK)

#
# Sources
#
SET(NeuroNav_SRCS 
  # module
  vtkNeuroNavLogic.cxx
  vtkNeuroNavGUI.cxx
  )

#
# Include dirs
#
INCLUDE_DIRECTORIES(
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${NeuroNav_SOURCE_DIR}
  ${NeuroNav_BINARY_DIR}
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
  ${IGT_SOURCE_DIR}
  ${IGT_BINARY_DIR}
#  ${NAVITRACK_INC}
  ${IGSTK_INCLUDE_DIRS}
  )

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(NeuroNav NeuroNav_TCL_SRCS "${NeuroNav_SRCS}" "")

#
# Loadable Module
#
GENERATELM(NeuroNav_SRCS NeuroNav.txt)

#
# Build the library
#
ADD_LIBRARY(NeuroNav
  ${NeuroNav_SRCS} 
  ${NeuroNav_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS NeuroNav
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(NeuroNav
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
  IGT
  ${IGSTK_LIB}
  )

#
# Configure include file
#
CONFIGURE_FILE(
  ${NeuroNav_SOURCE_DIR}/vtkNeuroNavConfigure.h.in 
  ${NeuroNav_BINARY_DIR}/vtkNeuroNavConfigure.h
  )
