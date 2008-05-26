project(MIMXCommon)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkCommonConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkCommonConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB templates "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
install(FILES 
  ${headers} 
  ${templates} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkCommonConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(MIMXCommon_SRCS 
  #mimxRegisterVersorRigidFilter.cxx  
  #mimxRegisterBSplineFilter.cxx 
  #mimxTransformIO.cxx 
  mimxCheckTypeOfFile.cxx
  vtkContainer.cxx
  vtkLinkedListWrapper.cxx
  vtkMimxCurve.cxx
  #vtkMimxSTLSource.cxx
  #vtkMimxUGrid.cxx
  vtkMimxBoundingBoxSource.cxx 
  vtkLocalAxesActor.cxx
  vtkPVAxesActor.cxx  
  vtkMimxSolidStructuredGridSource.cxx
  vtkMimxPlanarStructuredGridSource.cxx
  vtkMeshQualityClass.cxx
  vtkMimxApplyNodeElementNumbers.cxx
  vtkMimxCreateIntervertebralDiscElementSets.cxx
  vtkMimxMergeNodesPointAndCellData.cxx
  vtkMimxMeshSeed.cxx
  vtkMimxActorBase.cxx
  vtkMimxSurfacePolyDataActor.cxx
  vtkMimxUnstructuredGridActor.cxx
  )

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name mimxCommon)
add_library(${lib_name} 
  ${MIMXCommon_SRCS}
  )

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})
