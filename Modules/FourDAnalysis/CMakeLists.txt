PROJECT(FourDAnalysis)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)


# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${CommandLineModule_SOURCE_DIR}
  ${CommandLineModule_BINARY_DIR}
  ${FourDImage_SOURCE_DIR}
  ${FourDImage_BINARY_DIR}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  ${PYTHON_INCLUDE_PATH}
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkFourDAnalysisConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkFourDAnalysisConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkFourDAnalysisConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(FourDAnalysis_SRCS 
  vtkFourDAnalysisLogic.cxx
  vtkFourDAnalysisGUI.cxx
  vtkKWPlotGraph.cxx
  vtkIntensityCurves.cxx
  vtkMRMLCurveAnalysisNode.cxx
  vtkCurveAnalysisPythonInterface.cxx
  vtkMRMLDoubleArrayNode.cxx
  vtkMRMLXYPlotManagerNode.cxx
  vtkMRMLPlotNode.cxx
  vtkMRMLOrthogonalLinePlotNode.cxx
  vtkMRMLArrayPlotNode.cxx
  vtkSlicerXYPlotWidget.cxx
)


# --------------------------------------------------------------------------
# Python files (must be in Python/ directory)

set(FourDAnalysis_PYTHON_DIR Python)
set(FourDAnalysis_PYTHON_FILES
  FourDAnalysis.py
)



# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(FourDAnalysis FourDAnalysis_TCL_SRCS "${FourDAnalysis_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(FourDAnalysis_SRCS FourDAnalysis.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name FourDAnalysis)
add_library(${lib_name}
  ${FourDAnalysis_SRCS} 
  ${FourDAnalysis_TCL_SRCS}
  )
slicer3_set_modules_output_path(${lib_name})

find_library( CommandLineModuleLib CommandLineModule ${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR} )
if ( ${CommandLineModuleLib} MATCHES "NOTFOUND" )
   set ( CommandLineModuleLib CommandLineModule )
endif ( ${CommandLineModuleLib} MATCHES "NOTFOUND" )

find_library( VolumesLib Volumes ${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR} )
if ( ${VolumesLib} MATCHES "NOTFOUND" )
   set ( VolumesLib Volumes )
endif ( ${VolumesLib} MATCHES "NOTFOUND" )

find_library( FourDImageLib FourDImage ${Slicer3_DIR}/${Slicer3_INSTALL_MODULES_BIN_DIR} )
if ( ${FourDImageLib} MATCHES "NOTFOUND" )
   set ( FourDImageLib FourDImage )
endif ( ${FourDImageLib} MATCHES "NOTFOUND" )



target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${CommandLineModuleLib}
  ${ITK_LIBRARIES}
  ${CLP_MODULE}
  ${VolumesLib}
  ${FourDImageLib}
  ${PYTHON_LIBRARIES}
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )


slicer3_install_modules(${lib_name})


# --------------------------------------------------------------------------
# Install the Python files

foreach(file ${FourDAnalysis_PYTHON_FILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${FourDAnalysis_PYTHON_DIR}/${file}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${file}
    COPYONLY)
endforeach(file)

install(
  FILES ${FourDAnalysis_PYTHON_DIR}/${FourDAnalysis_PYTHON_FILES}
  DESTINATION ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
)

message("PYTHON FILES ARE INSTALLED IN : ${Slicer3_INSTALL_PLUGINS_BIN_DIR}")


