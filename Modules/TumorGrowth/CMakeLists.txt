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

include_directories(
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
  )

configure_file(
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

set(TumorGrowth_SRCS 
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
subdirs(DeformableMetric CommandLineApplication) 

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
add_library(${lib_name}
  ${TumorGrowth_SRCS}
  ${TumorGrowth_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  EMSegment
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  vtkTeem 
  Volumes
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
   subdirs(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# 
# Copy over Tcl files 
#

# Should those paths change, make sure to update 
#   - vtkTumorGrowthLogic.cxx

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")

foreach(file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tcl
  )

# 
# Copy over Script files 
#

file(GLOB SHFILES "${CMAKE_CURRENT_SOURCE_DIR}/Simulation/*.sh")
file(GLOB NHDRFILES "${CMAKE_CURRENT_SOURCE_DIR}/Simulation/*.nhdr")
file(GLOB RAWGZFILES "${CMAKE_CURRENT_SOURCE_DIR}/Simulation/*.raw.gz")
file(GLOB MHAFILES "${CMAKE_CURRENT_SOURCE_DIR}/Simulation/*.mha")

foreach(file 
    ${SHFILES} 
    ${NHDRFILES}
    ${RAWGZFILES}
    ${MHAFILES}
    )
  get_filename_component(filename "${file}" NAME)
  configure_file(
    ${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Simulation/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${SHFILES} 
  ${NHDRFILES} 
  ${RAWGZFILES}
  ${MHAFILES} 
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Simulation COMPONENT Development
  ) 

#
# copy over Testing files
#

file(GLOB MRMLFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.mrml")
file(GLOB NHDRFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.nhdr")
file(GLOB RAWGZFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.raw.gz")
file(GLOB LOGFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/*.log")
file(GLOB TESTFILES "${CMAKE_CURRENT_SOURCE_DIR}/Testing/test*Script")

foreach(file 
    ${MRMLFILES} 
    ${NHDRFILES}
    ${RAWGZFILES}
    ${LOGFILES}
    ${TESTFILES}
    )
  get_filename_component(filename "${file}" NAME)
  configure_file(
    ${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${MRMLFILES} 
  ${NHDRFILES} 
  ${RAWGZFILES}
  ${LOGFILES} 
  ${TESTFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  ) 
