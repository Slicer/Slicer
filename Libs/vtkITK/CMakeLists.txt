PROJECT(vtkITK)

# --------------------------------------------------------------------------
# Sources

SET(vtkITK_SRCS
    vtkITKArchetypeImageSeriesReader.cxx
    vtkITKArchetypeImageSeriesScalarReader.cxx
    vtkITKArchetypeImageSeriesVectorReader.cxx
)

# Abstract/pure virtual classes

#SET_SOURCE_FILES_PROPERTIES(
#  ABSTRACT
#  )

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
#  WRAP_EXCLUDE
#  )

# --------------------------------------------------------------------------
# Include dirs

INCLUDE_DIRECTORIES(
  ${ITK_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  )

# --------------------------------------------------------------------------
# Wrapping

# TODO - figure out how best to do the wrapping

# --------------------------------------------------------------------------
# Build the library

ADD_LIBRARY(vtkITK 
  ${vtkITK_SRCS} 
  )

TARGET_LINK_LIBRARIES(vtkITK 
  vtkCommon
  )


# --------------------------------------------------------------------------
# Testing (requires some of the examples)

IF(BUILD_TESTING)
  SUBDIRS(Testing)
ENDIF(BUILD_TESTING)
