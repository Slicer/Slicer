project(EMSegment)

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


find_package(GenerateLM REQUIRED)
if(GenerateLM_FOUND)
  include(${GenerateLM_USE_FILE})
endif(GenerateLM_FOUND)

# --------------------------------------------------------------------------
# Include dirs

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkEMSegmentConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentConfigure.h
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  ${EMSegmentMRML_SOURCE_DIR}
  ${EMSegmentMRML_BINARY_DIR}
  ${EMSegmentAlgorithm_SOURCE_DIR}
  ${EMSegmentAlgorithm_BINARY_DIR}
  ${EMSegmentRegistration_SOURCE_DIR}
  ${EMSegmentRegistration_BINARY_DIR}
  ${EMSegmentGraph_SOURCE_DIR}
  ${EMSegmentGraph_BINARY_DIR}
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Wizard/*.h"
  )

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tcl/*.tcl" )

foreach(file ${TCLFILES})
 configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)
# MESSAGE(STATUS  "${TCLFILES} ${CMAKE_CURRENT_SOURCE_DIR}")

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tcl
)


file(GLOB TASKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tasks/*.tcl" )

foreach(file ${TASKFILES})
 configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TASKFILES}
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tasks
)

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(EMSegment_SRCS 
  # module
  vtkEMSegmentGUI.cxx
  vtkEMSegmentLogic.cxx 
  # wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentParametersSetStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentIntensityImagesStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentAnatomicalStructureStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentSpatialPriorsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentNodeParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentIntensityDistributionsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentRegistrationParametersStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentRunSegmentationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentInputChannelsStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkEMSegmentPreProcessingStep.cxx
  )

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(EMSegment 
  EMSegment_TCL_SRCS 
  "${EMSegment_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(EMSegment_SRCS EMSegment.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name EMSegment)
add_library(${lib_name} 
  ${EMSegment_SRCS} 
  ${EMSegment_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  vtkSlicerVolumesModuleLogic
  EMSegmentMRML
  EMSegmentAlgorithm
  EMSegmentRegistration
  EMSegmentGraph
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
# Command line application

ADD_SUBDIRECTORY(CommandLineApplication)

# --------------------------------------------------------------------------
# Install the testing data

install(DIRECTORY
  Testing/TestData
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
