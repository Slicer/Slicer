project(LiverAblation)

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

# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkLiverAblationConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkLiverAblationConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Wizard/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/MRML/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Algorithm/*.txx"
  )
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkLiverAblationConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(LiverAblation_SRCS 
  # module
  vtkLiverAblationGUI.cxx
  vtkLiverAblationLogic.cxx 
  vtkLiverAblationMRMLManager.cxx 

  # wizard : Add wizard supporting programs here
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationOptimizationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationLoadingPreoperativeDataStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationCalibrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationPreplanningStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationPlanningStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationTrackingStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkLiverAblationNavigationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkKWGuideWidget.cxx

  # MRML: Add MRML supporting programs herer
  # Algorithm  : Add  algorithm related programs here
)

#set_source_files_properties(
  # Algorithm sources (temporary exclusion)
#  WRAP_EXCLUDE
#  )
  

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(LiverAblation LiverAblation_TCL_SRCS "${LiverAblation_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(LiverAblation_SRCS LiverAblation.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name LiverAblation)
add_library(${lib_name} 
  ${LiverAblation_SRCS} 
  ${LiverAblation_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

install(DIRECTORY
  Testing/TestData
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
