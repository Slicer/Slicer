PROJECT(IA_FEMesh)

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
# Include dirs
#
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${MIMXCommon_SOURCE_DIR} 
  ${MIMXCommon_BINARY_DIR}
  ${MIMXAdapter_SOURCE_DIR} 
  ${MIMXAdapter_BINARY_DIR} 
  ${MIMXMesh_SOURCE_DIR} 
  ${MIMXMesh_BINARY_DIR} 
  ${MIMXWidgets_SOURCE_DIR} 
  ${MIMXWidgets_BINARY_DIR} 
  ${BUILDINGBLOCK_SOURCE_DIR} 
  ${BUILDINGBLOCK_BINARY_DIR} 
 ) 
 
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkIA_FEMeshConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkIA_FEMeshConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkIA_FEMeshConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )
  
 # until slicer goes to vtk5.2, we need to specially define a portion of the Verdict library to have 
 # advanced mesh rendering functions 
  

SET(LIB_NAME IA_FEMesh)
SET(IA_FEMesh_SRCS 
  vtkIA_FEMeshGUI.cxx
  vtkIA_FEMeshLogic.cxx
  vtkMRMLIA_FEMeshNode.cxx
  #vtkIA_FEMeshMRMLNotebook.cxx
  #vtkMeshQualityExtended.cxx
  #${VERDICT_SOURCES}
  )
   

#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
#INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#VTK_WRAP_TCL3(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(IA_FEMesh 
   IA_FEMesh_TCL_SRCS 
  "${IA_FEMesh_SRCS}" "")
  
  
#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(IA_FEMesh_SRCS 
 IA_FEMesh.txt
 )
  

# Create the library. The library is built in static mode for convenience. 
# Check the 'Callbacks' example for more information about building it in
# shared mode, i.e. without the STATIC keyword (Win32 compilers requires
# an additional header file to setup DLL export symbols correctly).

ADD_LIBRARY(${LIB_NAME} ${IA_FEMesh_TCL_SRCS} ${IA_FEMesh_SRCS})

slicer3_set_modules_output_path(${LIB_NAME})

TARGET_LINK_LIBRARIES(${LIB_NAME}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES} 
  mimxWidgets
  mimxMeshIO
  mimxFilter
  mimxCommon
  BuildingBlock
  )

slicer3_install_modules(${LIB_NAME})
