project(EMSegment)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


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
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
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
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)
# MESSAGE(STATUS  "${TCLFILES} ${CMAKE_CURRENT_SOURCE_DIR}")

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tcl
)


file(GLOB TASKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Tasks/*.tcl" )

foreach(file ${TASKFILES})
 configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${TASKFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Tasks
)

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkEMSegmentConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
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
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
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
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES
    ${Slicer3_LIBRARY_PROPERTIES}
  )
ENDIF(Slicer3_LIBRARY_PROPERTIES)

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
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
