project(ExtractSubvolumeROI)

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

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Wizard
  ${CMAKE_CURRENT_SOURCE_DIR}/Filters
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${vtkTeem_SOURCE_DIR} 
  ${vtkTeem_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  )

# --------------------------------------------------------------------------
# Install support files

file(GLOB TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.tcl")
foreach(file ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR}/${PROJECT_NAME}
  )

# logo
file(GLOB IMAGEFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "ImageData/*.png")
foreach(file ${IMAGEFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/${file}
    COPYONLY
    )
endforeach(file)

install(
  FILES ${IMAGEFILES}
  DESTINATION ${Slicer3_INSTALL_MODULES_SHARE_DIR}/${PROJECT_NAME}/ImageData
  )

#configure_file(
#  ${ExtractSubvolumeROI_SOURCE_DIR}/vtkPichonFastMarchingConfigure.h.in
#  ${ExtractSubvolumeROI_BINARY_DIR}/vtkPichonFastMarchingConfigure.h
#  )
  
#file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
#install(
#  FILES ${headers} 
#  "${CMAKE_CURRENT_BINARY_DIR}/vtkPichonFastMarchingConfigure.h"
#  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
#  )

#include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
#set(LIB_NAME PichonFastMarching)
#set(LIB_SRCS vtkPichonFastMarching.cxx vtkPichonFastMarchingPDF.cxx)
#vtk_wrap_tcl3(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

#add_library(${LIB_NAME} ${LIB_SRCS} ${LIB_TCL_SRCS})

#target_link_libraries(${LIB_NAME}
#  ${Slicer3_Libs_LIBRARIES}
#  ${Slicer3_Base_LIBRARIES}
#  ${KWWidgets_LIBRARIES}
#  )

#install(TARGETS ${LIB_NAME}
#  RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
#  LIBRARY DESTINATION bin COMPONENT RuntimeLibraries
#  ARCHIVE DESTINATION bin COMPONENT Development
#  )
