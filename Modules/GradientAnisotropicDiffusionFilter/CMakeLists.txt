project(GradientAnisotropicDiffusionFilter)

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# Sources

set(GradientAnisotropicDiffusionFilter_SRCS
  vtkGradientAnisotropicDiffusionFilterLogic.cxx
  vtkGradientAnisotropicDiffusionFilterGUI.cxx
  vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx
  )

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#ABSTRACT "TRUE"
#)

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
#vtkModule.cxx
#PROPERTIES
#WRAP_EXCLUDE "TRUE"
#)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
  ${Slicer3_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Wrapping

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(GradientAnisotropicDiffusionFilter 
  GradientAnisotropicDiffusionFilter_TCL_SRCS 
  "${GradientAnisotropicDiffusionFilter_SRCS}" "")

# --------------------------------------------------------------------------
# Build the library

add_library(GradientAnisotropicDiffusionFilter
  ${GradientAnisotropicDiffusionFilter_SRCS}
  ${GradientAnisotropicDiffusionFilter_TCL_SRCS}
  #loadable module support
  GradientAnisotropicDiffusionFilterModuleLib.cxx
  )

if(BUILD_SHARED_LIBS)
  install(TARGETS GradientAnisotropicDiffusionFilter
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

target_link_libraries(GradientAnisotropicDiffusionFilter
  ${Slicer3_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${ITK_LIBRARIES}
  )

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

if(BUILD_TESTING)
  subdirs(Testing)
endif(BUILD_TESTING)

configure_file(
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h.in 
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}/vtkGradientAnisotropicDiffusionFilterConfigure.h
  )
