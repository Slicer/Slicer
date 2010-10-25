project(IGTNavigation)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkIGTNavigationConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkIGTNavigationConfigure.h
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
  "${CMAKE_CURRENT_BINARY_DIR}/vtkIGTNavigationConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(IGTNavigation_SRCS 
  # module
  vtkIGTNavigationGUI.cxx
  vtkIGTNavigationLogic.cxx 
  vtkIGTNavigationMRMLManager.cxx 
  # wizard : Add wizard supporting programs here
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationInitializationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationLoadingPreoperativeDataStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationCalibrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationRegistrationStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkIGTNavigationIntraoperativeProcedureStep.cxx

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
vtk_wrap_tcl3(IGTNavigation IGTNavigation_TCL_SRCS "${IGTNavigation_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(IGTNavigation_SRCS IGTNavigation.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name IGTNavigation)
add_library(${lib_name} 
  ${IGTNavigation_SRCS} 
  ${IGTNavigation_TCL_SRCS}
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
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

install(DIRECTORY
  Testing/TestData
  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
  PATTERN ".svn" EXCLUDE
  )
