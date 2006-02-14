PROJECT(MRML)

# --------------------------------------------------------------------------
# Sources

SET(MRML_SRCS
    vtkMRMLScene.cxx
    vtkMRMLParser.cxx
    vtkMRMLNode.cxx
    vtkMRMLVolumeNode.cxx
)

# Abstract/pure virtual classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  ABSTRACT
  )

# Helper classes

SET_SOURCE_FILES_PROPERTIES(
  vtkMRMLNode.cxx 
  WRAP_EXCLUDE
  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${VTK_INCLUDE_DIR}
  ${Slicer3_SOURCE_DIR}/Libs/vtkITK
  )



# --------------------------------------------------------------------------
# Wrapping

# TODO - figure out how best to do the wrapping

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(MRML 
  ${MRML_SRCS} 
  )

TARGET_LINK_LIBRARIES(MRML 
  vtkCommon
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)
