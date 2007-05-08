PROJECT (EMSegmentCommandLineExecutable)

INCLUDE_DIRECTORIES ( 
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_SOURCE_DIR}/MRML
  ${EMSegment_SOURCE_DIR}/Algorithm
  ${VTK_INCLUDE_DIR}
  )

ADD_EXECUTABLE(
  EMSegmentCommandLine
  vtkEMSegmentCommandLine.cxx
  )
TARGET_LINK_LIBRARIES(
  EMSegmentCommandLine
  EMSegment
  vtkCommon
  )


