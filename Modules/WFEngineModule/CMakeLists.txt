PROJECT(WFEngineModule)

OPTION(WFENGINEMODULE "Do you want to build the WFEngineModule?")
IF(WFENGINEMODULE)
  
# --------------------------------------------------------------------------
# Sources

SET(WFEngineModule_SRCS
  vtkWFEngineModuleLogic.cxx
  vtkWFEngineModuleGUI.cxx
  vtkMRMLWFEngineModuleNode.cxx
  vtkKWMyWizardWidget.cxx  
  vtkWFEngineHandler.cxx
  vtkWFEngineEventHandler.cxx
)
# vtkKWMyWizardWorkflow.cxx - obsolent


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

# Find the WFEngine Sources and libraries
INCLUDE(FindXercesC.cmake)

#Build the WFEngine Library
SET(WFENGINE_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/WFEngine)
SET(WFENGINE_LIBRARY "")
SUBDIRS(WFEngine)

SET(WFENGINE_INCLUDE_DIR ${WFENGINE_BASE_DIRECTORY} ${WFENGINE_BASE_DIRECTORY}/engine
                            ${WFENGINE_BASE_DIRECTORY}/interfaces ${WFENGINE_BASE_DIRECTORY}/interfaces/direct 
                            ${WFENGINE_BASE_DIRECTORY}/xmlManager)
                            
SET(WFENGINE_LIBRARY ${LIBRARY_OUTPUT_PATH}libWFDirectInterface.so)                                          
                            
# --------------------------------------------------------------------------
# Include dirs

SET(WFEngineModule_SOURCE_DIR ${WFEngineModule_SOURCE_DIR} ${WFENGINE_INCLUDE_DIR} ${XERCESC_INCLUDE_DIR})

INCLUDE_DIRECTORIES(
  ${WFEngineModule_SOURCE_DIR}
  ${WFEngineModule_BINARY_DIR}
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${vtkITK_SOURCE_DIR}
  ${vtkITK_BINARY_DIR}
  ${ITK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(WFEngineModule WFEngineModule_TCL_SRCS "${WFEngineModule_SRCS}" "")

# --------------------------------------------------------------------------
# Sources which shall not be wrapped

SET(WFEngineModule_SRCS ${WFEngineModule_SRCS} WFStateConverter.cxx)

 
# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(WFEngineModule
  ${WFEngineModule_SRCS}
  ${WFEngineModule_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} WFEngineModule)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(WFEngineModule
  SlicerBaseLogic 
  SlicerBaseGUI 
  MRML
  vtkITK
  vtkCommonTCL
  vtkImagingTCL
  vtkFilteringTCL
  vtkIOTCL
  ITKAlgorithms
  ITKNumerics 
  ITKCommon
  ITKBasicFilters
  ITKNumerics
  ITKStatistics
  ITKBasicFilters
  ITKIO
  ITKDICOMParser
  ${KWWidgets_LIBRARIES}
  ${WFENGINE_LIBRARY}
  ${XERCESC_LIBRARY})


SET(WFENGINE_LINK_LIB ${WFENGINE_LIBRARY})

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

CONFIGURE_FILE(
  ${WFEngineModule_SOURCE_DIR}/vtkWFEngineModuleConfigure.h.in 
  ${WFEngineModule_BINARY_DIR}/vtkWFEngineModuleConfigure.h
)
ENDIF(WFENGINEMODULE)
