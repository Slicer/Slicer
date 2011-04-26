PROJECT(OpenIGTLinkIF)

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
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${OpenIGTLink_INCLUDE_DIRS}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkOpenIGTLinkIFConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkOpenIGTLinkIFConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkOpenIGTLinkIFConfigure.h"
  DESTINATION ${Slicer_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )


# --------------------------------------------------------------------------
# Sources

set(OpenIGTLinkIF_SRCS 
  vtkOpenIGTLinkIFLogic.cxx
  vtkOpenIGTLinkIFGUI.cxx
  #vtkIGTLConnector.cxx
  vtkIGTLCircularBuffer.cxx
  vtkIGTLToMRMLBase.cxx
  vtkIGTLToMRMLLinearTransform.cxx
  vtkIGTLToMRMLImage.cxx
  vtkIGTLToMRMLPosition.cxx
  vtkMRMLIGTLConnectorNode.cxx
  vtkIGTLRemoteDataListWindow.cxx
  vtkMRMLIGTLQueryNode.cxx
  vtkMRMLImageMetaListNode.cxx
  vtkIGTLToMRMLImageMetaList.cxx
#  vtkMRMLIGTLImageMetaListQueryNode.cxx
  vtkIGTLToMRMLTrackingData.cxx
  vtkMRMLIGTLTrackingDataBundleNode.cxx
  vtkIGTLTrackingDataControllerWindow.cxx
)

# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(OpenIGTLinkIF OpenIGTLinkIF_TCL_SRCS "${OpenIGTLinkIF_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(OpenIGTLinkIF_SRCS OpenIGTLinkIF.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name OpenIGTLinkIF)

add_library(${lib_name}
  ${OpenIGTLinkIF_SRCS} 
  ${OpenIGTLinkIF_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})


target_link_libraries(${lib_name}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  ${OpenIGTLink_LIBRARIES}
  )
slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
   ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
