PROJECT(QdecModule)

SET (MODULE QdecModule)



# --------------------------------------------------------------------------
# Sources

SET(${MODULE}_SRCS
  vtk${MODULE}Logic.cxx
  vtk${MODULE}GUI.cxx
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
# MESSAGE(STATUS "QDecModle: SandBox dir = ${SandBox_DIR}, Qdec binary dir = ${Qdec_BINARY_DIR}")
INCLUDE_DIRECTORIES(
  ${${MODULE}_SOURCE_DIR}
  ${${MODULE}_BINARY_DIR}
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
  ${SandBox_DIR}/Qdec
  ${SandBox_DIR}-build/Qdec
  )


# --------------------------------------------------------------------------
# Wrapping

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
VTK_WRAP_TCL3(${MODULE} ${MODULE}_TCL_SRCS "${${MODULE}_SRCS}" "")

# MESSAGE(STATUS "QdecModule: MODULE = ${MODULE}\nSandbox dir = ${SandBox_DIR}\nadding link dir = ${SandBox_DIR}-build/bin")

# --------------------------------------------------------------------------
# Build the library

# differentiate between the Module's library file and the Slicer3-lib NAMIC SandBox Qdec library file
LINK_DIRECTORIES(${SandBox_DIR}-build/bin)

ADD_LIBRARY(${MODULE} SHARED
  ${${MODULE}_SRCS}
  ${${MODULE}_TCL_SRCS}
  )

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} ${MODULE})
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(${MODULE}
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



# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
#  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)

# MESSAGE(STATUS "Configuring QdecModule: to bind dir: ${${MODULE}_BINARY_DIR}")
CONFIGURE_FILE(
  ${${MODULE}_SOURCE_DIR}/vtkQdecModuleConfigure.h.in 
  ${${MODULE}_BINARY_DIR}/vtkQdecModuleConfigure.h
)
