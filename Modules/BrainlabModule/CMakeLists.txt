project(BrainlabModule)

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

# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${OpenIGTLink_INCLUDE_DIRS}
  ${OpenIGTLinkIF_SOURCE_DIR}
  ${OpenIGTLinkIF_BINARY_DIR}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkBrainlabModuleConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkBrainlabModuleConfigure.h
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
  "${CMAKE_CURRENT_BINARY_DIR}/vtkBrainlabModuleConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(BrainlabModule_SRCS 
  # module
  vtkBrainlabModuleGUI.cxx
  vtkBrainlabModuleLogic.cxx 
  vtkBrainlabModuleMRMLManager.cxx 

  # wizard : Add wizard supporting programs here
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkBrainlabModuleStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkBrainlabModuleLoadingDataStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkBrainlabModuleConnectionStep.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard/vtkBrainlabModuleNavigationStep.cxx

  # MRML: Add MRML supporting programs herer
  # Algorithm  : Add  algorithm related programs here
)

# set_source_files_properties(
# Algorithm sources (temporary exclusion)
# WRAP_EXCLUDE
# )
  

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(BrainlabModule BrainlabModule_TCL_SRCS "${BrainlabModule_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(BrainlabModule_SRCS BrainlabModule.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name BrainlabModule)
add_library(${lib_name} 
  ${BrainlabModule_SRCS} 
  ${BrainlabModule_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  ${OpenIGTLink_LIBRARIES}
  OpenIGTLinkIF
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

# --------------------------------------------------------------------------
# Install the testing data

# install(DIRECTORY
#  Testing/TestData
#  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#  PATTERN ".svn" EXCLUDE
#  )
