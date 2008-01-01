PROJECT(ProstateNav)

IF(NOT USE_IGSTK)
 ADD_DEFINITIONS(-DIGSTK_OFF)
ENDIF(NOT USE_IGSTK)

#
# Sources
#
SET(ProstateNav_SRCS 
  # module
  vtkProstateNavLogic.cxx
  vtkProstateNavGUI.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavConfigurationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavScanControlStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavCalibrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavTargetingStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkProstateNavManualControlStep.cxx
  )

IF(USE_NAVITRACK)
  SET(ProstateNav_SRCS
    ${ProstateNav_SRCS}
    vtkProstateNavDataStream.cxx
    )
ENDIF(USE_NAVITRACK)

#
# Include dirs
#
MESSAGE("\n\n navitrack inc dir = ${NAVITRACK_INC}")
MESSAGE("\n\n compile option USE_NAVITRACK = ${USE_NAVITRACK}")

INCLUDE_DIRECTORIES(
  ${ModulesBaseClasses_SOURCE_DIR}
  ${ModulesBaseClasses_BINARY_DIR}
  ${ProstateNav_SOURCE_DIR}
  ${ProstateNav_BINARY_DIR}
  ${ProstateNav_SOURCE_DIR}/Wizard
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
  ${NAVITRACK_INC}
  ${IGSTK_INCLUDE_DIRS}
  )

#
# Wrapping
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(ProstateNav ProstateNav_TCL_SRCS "${ProstateNav_SRCS}" "")

#
# Build the library
#
ADD_LIBRARY(ProstateNav
  ${ProstateNav_SRCS} 
  ${ProstateNav_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} ProstateNav)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(ProstateNav
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
  ${ProstateNav_SOURCE_DIR}/vtkProstateNavConfigure.h.in 
  ${ProstateNav_BINARY_DIR}/vtkProstateNavConfigure.h
  )
