PROJECT(FourDAnalysis)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
# --------------------------------------------------------------------------
# Option: use Scipy for curve fitting (SciPy must be installed)

option(FourDAnalysis_USE_SCIPY "Use SciPy for curve fitting." OFF)

if (FourDAnalysis_USE_SCIPY)
  ADD_DEFINITIONS(-DFourDAnalysis_USE_SCIPY)
endif (FourDAnalysis_USE_SCIPY)

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
  ${vtkSlicerVolumesModuleLogic_SOURCE_DIR}
  ${vtkSlicerVolumesModuleLogic_BINARY_DIR}
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

#file(GLOB headers 
#  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
#  )

set(headers
  vtkFourDAnalysisGUI.h
  vtkFourDAnalysisLogic.h
  vtkFourDAnalysisWin32Header.h
  vtkIntensityCurves.h
  vtkMRMLCurveAnalysisNode.h
  )

if (FourDAnalysis_USE_SCIPY)
   set(headers
     ${headers}
     vtkCurveAnalysisPythonInterface.h
     )
endif (FourDAnalysis_USE_SCIPY)

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
  vtkIntensityCurves.cxx
  vtkMRMLCurveAnalysisNode.cxx
)

if (FourDAnalysis_USE_SCIPY)
set (FourDAnalysis_SRCS
  ${FourDAnalysis_SRCS}
  vtkCurveAnalysisPythonInterface.cxx
)
endif (FourDAnalysis_USE_SCIPY)


# --------------------------------------------------------------------------
# Python files (must be in Python/ directory)

if (FourDAnalysis_USE_SCIPY)
  set(FourDAnalysis_PYTHON_DIR Python)
  set(FourDAnalysis_PYTHON_FILES
    FourDAnalysis.py
  )
endif (FourDAnalysis_USE_SCIPY)


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
  vtkSlicerVolumesModuleLogic
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

if (FourDAnalysis_USE_SCIPY)
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

endif (FourDAnalysis_USE_SCIPY)



