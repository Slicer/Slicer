PROJECT(QdecModule)


# --------------------------------------------------------------------------
# Sources

SET(QdecModule_SRCS
  vtkQdecModuleLogic.cxx
  vtkQdecModuleGUI.cxx
  vtkGDFReader.cxx
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
# MESSAGE(STATUS "QDecModle: Qdec binary dir = ${Qdec_BINARY_DIR}")
INCLUDE_DIRECTORIES(
  ${QdecModule_SOURCE_DIR}
  ${QdecModule_BINARY_DIR}
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
  ${Qdec_BINARY_DIR}
  ${Qdec_SOURCE_DIR}
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(QdecModule QdecModule_TCL_SRCS "${QdecModule_SRCS}" "")

# MESSAGE(STATUS "QdecModule: MODULE = QdecModule\nSandbox dir = ${SandBox_DIR}\nadding link dir = ${SandBox_DIR}-build/bin")

# --------------------------------------------------------------------------
# Loadable Module support, do after TCL Wrapping

GENERATELM(QdecMOdule_SRCS QdecModule.txt)

# --------------------------------------------------------------------------
# Build the library

# differentiate between the Module's library file and the Slicer3-lib NAMIC SandBox Qdec library file
LINK_DIRECTORIES(${Qdec_BINARY_DIR})

ADD_LIBRARY(QdecModule 
  ${QdecModule_SRCS}
  ${QdecModule_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS QdecModule
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(QdecModule
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
  Qdec
  )

IF (USE_PYTHON)
  ADD_DEFINITIONS(-DUSE_PYTHON)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
ENDIF(USE_PYTHON)

# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
#  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

# MESSAGE(STATUS "Configuring QdecModule: to bind dir: ${QdecModule_BINARY_DIR}")
CONFIGURE_FILE(
  ${QdecModule_SOURCE_DIR}/vtkQdecModuleConfigure.h.in 
  ${QdecModule_BINARY_DIR}/vtkQdecModuleConfigure.h
)
