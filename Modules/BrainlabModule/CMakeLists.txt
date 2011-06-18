project(BrainlabModule)

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

# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
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
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
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
#slicer3_set_modules_output_path(${lib_name})
SET_TARGET_PROPERTIES(${lib_name} PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_BIN_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_MODULES_LIB_DIR}"
  )

target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  ${ITK_LIBRARIES}
  ${OpenIGTLink_LIBRARIES}
  OpenIGTLinkIF
  )

#slicer3_install_modules(${lib_name})
INSTALL(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
ENDIF()

# --------------------------------------------------------------------------
# Install the testing data

# install(DIRECTORY
#  Testing/TestData
#  DESTINATION ${Slicer_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/Testing COMPONENT Development
#  PATTERN ".svn" EXCLUDE
#  )
