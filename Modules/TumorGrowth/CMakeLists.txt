project(TumorGrowth)

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
# Include dirs

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${EMSegment_SOURCE_DIR}/Registration
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  ${vtkTeem_SOURCE_DIR} 
  ${vtkTeem_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}

#  ${SlicerBase_SOURCE_DIR}
#  ${SlicerBase_BINARY_DIR}
#  ${SlicerBaseLogic_SOURCE_DIR}
#  ${SlicerBaseLogic_BINARY_DIR}
#  ${SlicerBaseGUI_SOURCE_DIR}
#  ${SlicerBaseGUI_BINARY_DIR}
#  ${vtkITK_SOURCE_DIR}
#  ${vtkITK_BINARY_DIR}
#  ${ITK_INCLUDE_DIR}
#  ${VTK_INCLUDE_DIR}
#  ${MRML_SOURCE_DIR}
#  ${MRML_BINARY_DIR}

  )

CONFIGURE_FILE(
  ${TumorGrowth_SOURCE_DIR}/vtkTumorGrowthConfigure.h.in 
  ${TumorGrowth_BINARY_DIR}/vtkTumorGrowthConfigure.h
  )


file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkTumorGrowthConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

SET(TumorGrowth_SRCS 
  # module
  vtkTumorGrowthGUI.cxx
  vtkTumorGrowthLogic.cxx 
  # Filters
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageRectangularSource.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageIslandFilter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageHistogramNormalization.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageGCR.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageKilianDistanceTransform.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageSumOverVoxels.cxx 
  # wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthSelectScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthFirstScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthROIStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthSegmentationStep.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthSecondScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthTypeStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkTumorGrowthAnalysisStep.cxx
  # MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLTumorGrowthNode.cxx
)

# Enders work 
SUBDIRS(DeformableMetric CommandLineApplication) 


# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#WRAP_EXCLUDE "TRUE"
#)

# --------------------------------------------------------------------------
# Wrapping
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(TumorGrowth TumorGrowth_TCL_SRCS "${TumorGrowth_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(TumorGrowth_SRCS TumorGrowth.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name TumorGrowth)

#
# Build the library
#

add_library(${lib_name}
  ${TumorGrowth_SRCS}
  ${TumorGrowth_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})


# From old Cmake file 
# IF(BUILD_SHARED_LIBS)
#  INSTALL(TARGETS TumorGrowth
#      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
#      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
#      ARCHIVE DESTINATION lib COMPONENT Development)
# ENDIF(BUILD_SHARED_LIBS)






target_link_libraries(${lib_name}
  EMSegment
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}


  # SlicerBaseLogic 
  # SlicerBaseGUI 
  # MRML
  # vtkITK
  # vtkCommonTCL
  # vtkImagingTCL
  # vtkFilteringTCL
  # vtkIOTCL
  # ITKAlgorithms
  # ITKNumerics 
  # ITKCommon
  # ITKBasicFilters
  # ITKNumerics
  # ITKStatistics
  # ITKBasicFilters
  # ITKIO
  # ITKDICOMParser
  vtkTeem 
  Volumes
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
   subdirs(Testing)
endif(BUILD_TESTING)

#
# copy over the tcl files
#
SET (TCL_DIR ${TumorGrowth_BINARY_DIR}/tcl)
IF (NOT EXISTS ${TCL_DIR})
  FILE(MAKE_DIRECTORY ${TCL_DIR})
ENDIF (NOT EXISTS ${TCL_DIR})

SET(TCLFILES
  TumorGrowthFct.tcl 
  TumorGrowthReg.tcl
)

FOREACH(tclfile ${TCLFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/TumorGrowth/tcl/${tclfile}
    ${TCL_DIR}/${tclfile}
    COPYONLY
  )
ENDFOREACH(tclfile)

# 
# Copy over Script files 
#

SET (SIMULATION_DIR ${TumorGrowth_BINARY_DIR}/Simulation)

IF (NOT EXISTS ${SIMULATION_DIR})
  FILE(MAKE_DIRECTORY ${SIMULATION_DIR})
ENDIF (NOT EXISTS ${SIMULATION_DIR})

SET(SIMULATIONFILES
  create_artificial_growth_deformation.sh
  original.nhdr
  original.raw.gz
  copy-deformation_vectors_base.mha
)

FOREACH(simfile ${SIMULATIONFILES})

CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/TumorGrowth/Simulation/${simfile}
    ${SIMULATION_DIR}/${simfile}
    COPYONLY
  )
ENDFOREACH(simfile)


#
# copy over test files
#
SET (TEST_DIR ${TumorGrowth_BINARY_DIR}/Testing)
IF (NOT EXISTS ${TEST_DIR})
  FILE(MAKE_DIRECTORY ${TEST_DIR})
ENDIF (NOT EXISTS ${TEST_DIR})

SET(TESTFILES
  scan1.nhdr
  scan1.raw.gz
  scan2.nhdr
  scan2.raw.gz
  TG_scan1_Segment.nhdr
  TG_scan1_Segment.raw.gz 
  TG_scan1_SuperSampled.nhdr 
  TG_scan1_SuperSampled.raw.gz
  TG_scan2_norm.nhdr
  TG_scan2_norm.raw.gz
  test.mrml
  testScript
  testDemonsScript
  GUI-AnalysisOutcome.log
  Script-AnalysisOutcome.log
)

FOREACH(testfile ${TESTFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/TumorGrowth/Testing/${testfile}
    ${TEST_DIR}/${testfile}
    COPYONLY
  )
ENDFOREACH(testfile)


