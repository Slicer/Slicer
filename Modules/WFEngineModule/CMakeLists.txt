project(WFEngineModule)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_modules_install_prefix()
endif(NOT Slicer3_SOURCE_DIR)

slicer3_set_default_modules_output_path()

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkWFEngineModuleConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkWFEngineModuleConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkWFEngineModuleConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(WFEngineModule_SRCS
  vtkWFEngineModuleLogic.cxx
  vtkWFEngineModuleGUI.cxx
  vtkMRMLWFEngineModuleNode.cxx
  vtkKWMyWizardWidget.cxx  
  vtkWFEngineHandler.cxx
  vtkWFEngineEventHandler.cxx
  WFStateConverter.cxx
  )

set_source_files_properties(
  WFStateConverter.cxx
  PROPERTIES
  WRAP_EXCLUDE "TRUE"
  )

# --------------------------------------------------------------------------
# Xerces and WFEngine

# Find the WFEngine Sources and libraries
include(FindXercesC.cmake)

# Build the WFEngine Library

set(WFENGINE_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/WFEngine)
include_directories(
  ${WFENGINE_BASE_DIRECTORY} 
  ${WFENGINE_BASE_DIRECTORY}/engine
  ${WFENGINE_BASE_DIRECTORY}/interfaces 
  ${WFENGINE_BASE_DIRECTORY}/interfaces/direct 
  ${WFENGINE_BASE_DIRECTORY}/xmlManager
  )
if(XERCESC_INCLUDE_DIR)
  include_directories(${XERCESC_INCLUDE_DIR})
endif(XERCESC_INCLUDE_DIR)

set(WFENGINE_LIBRARY "")
subdirs(WFEngine)
set(WFENGINE_LIBRARY ${LIBRARY_OUTPUT_PATH}libWFDirectInterface.so)

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(WFEngineModule WFEngineModule_TCL_SRCS "${WFEngineModule_SRCS}" "")

# --------------------------------------------------------------------------
# Add Loadable Module support

generatelm(WFEngineModule_SRCS WFEngineModule.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name WFEngineModule)
add_library(${lib_name}
  ${WFEngineModule_SRCS}
  ${WFEngineModule_TCL_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )
if(WFENGINE_LIBRARY)
  target_link_libraries(${lib_name} ${WFENGINE_LIBRARY})
endif(WFENGINE_LIBRARY)
if(XERCESC_LIBRARY)
  target_link_libraries(${lib_name} ${XERCESC_LIBRARY})
endif(XERCESC_LIBRARY)

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Install support files

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/wfConfig.xml
  ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/wfConfig.xml
  COPYONLY
  )

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/wfConfig.xml
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}
  )

file(GLOB WORKFLOWFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "workflows/*.xml")
foreach(file ${WORKFLOWFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${WORKFLOWFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/workflows
  )
