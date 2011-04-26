project(ChangeTracker)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkTeem_SOURCE_DIR} 
  ${vtkTeem_BINARY_DIR}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  # AF: this is needed to wrap the command line module (Linear registration)
  ${CommandLineModule_BINARY_DIR}
  ${CommandLineModule_SOURCE_DIR}
  )

configure_file(
  ${ChangeTracker_SOURCE_DIR}/vtkChangeTrackerConfigure.h.in 
  ${ChangeTracker_BINARY_DIR}/vtkChangeTrackerConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkChangeTrackerConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(ChangeTracker_SRCS 
  # module
  vtkChangeTrackerGUI.cxx
  vtkChangeTrackerLogic.cxx 
  # Filters
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageRectangularSource.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageIslandFilter.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageHistogramNormalization.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageGCR.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageKilianDistanceTransform.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters/vtkImageSumOverVoxels.cxx 
  # wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerStep.cxx
  # AF: will commit next merged version of first+second steps
#  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerScanSelectionStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerSelectScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerFirstScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerROIStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerSegmentationStep.cxx
  # ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerSecondScanStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerTypeStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkChangeTrackerAnalysisStep.cxx
  # MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLChangeTrackerNode.cxx
)

# Enders work 
SET(changetracker_libs
  DeformableMetric
  CommandLineApplication
  DiffeomorphicDemonsRegistration
  )
FOREACH(lib ${changetracker_libs})
  ADD_SUBDIRECTORY(${lib})
ENDFOREACH(lib)

# Tools for diameter estimation
#ADD_SUBDIRECTORY(TumorDiameterEstimation)

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
vtk_wrap_tcl3(ChangeTracker ChangeTracker_TCL_SRCS "${ChangeTracker_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(ChangeTracker_SRCS ChangeTracker.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name ChangeTracker)
add_library(${lib_name}
  ${ChangeTracker_SRCS}
  ${ChangeTracker_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  vtkTeem
  vtkSlicerVolumesModuleLogic
  Volumes
  )

# Apply user-defined properties to the library target.
IF(Slicer_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
     ${Slicer_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer_LIBRARY_PROPERTIES)

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
   ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install support files

# 
# Copy over Tcl files 
#

# Should those paths change, make sure to update 
#   - vtkChangeTrackerLogic.cxx

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl")

foreach(file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tcl
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
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Simulation/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${SHFILES} 
  ${NHDRFILES} 
  ${RAWGZFILES}
  ${MHAFILES} 
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Simulation COMPONENT Development
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
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing/${filename}
    COPYONLY)
endforeach(file)

install(FILES 
  ${MRMLFILES} 
  ${NHDRFILES} 
  ${RAWGZFILES}
  ${LOGFILES} 
  ${TESTFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  ) 
  
#
# Copy over BSF logo (and any new logos that might be added in the future)
#
file(GLOB IMAGEFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "ImageData/*.png")
foreach(file ${IMAGEFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)
