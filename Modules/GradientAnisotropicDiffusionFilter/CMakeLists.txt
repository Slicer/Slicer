project(GradientAnisotropicDiffusionFilter)

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

# IF(Slicer3_USE_QT)
#   set(include_dirs ${include_dirs} ${QT_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR})
#   QT4_WRAP_CPP(MOC_SRCS QtGADModule.h)
#   SET(SRCS ${SRCS} ${MOC_SRCS} 
#            QtGADModule.cxx QtGADModule.h)
#   SET(QT_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} QVTK)
# ENDIF(Slicer3_USE_QT)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h
  )

file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(GradientAnisotropicDiffusionFilter_SRCS
  vtkGradientAnisotropicDiffusionFilterLogic.cxx
  vtkGradientAnisotropicDiffusionFilterGUI.cxx
  vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx
  ${SRCS}
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

# SET_SOURCE_FILES_PROPERTIES(
#   QtGADModule.cxx 
#   moc_QtGADModule.cxx 
#   QtGADModule.h
#   WRAP_EXCLUDE
#   )


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(GradientAnisotropicDiffusionFilter 
  GradientAnisotropicDiffusionFilter_TCL_SRCS 
  "${GradientAnisotropicDiffusionFilter_SRCS}" "")

#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(GradientAnisotropicDiffusionFilter_SRCS 
  GradientAnisotropicDiffusionFilter.xml)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name GradientAnisotropicDiffusionFilter)
add_library(${lib_name}
  ${GradientAnisotropicDiffusionFilter_SRCS}
  ${GradientAnisotropicDiffusionFilter_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
#   ${QT_LIBRARIES}
  ${ITK_LIBRARIES}
  )

slicer3_install_modules(${lib_name})

# --------------------------------------------------------------------------
# Testing

if(BUILD_TESTING)
#  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
