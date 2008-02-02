PROJECT( MimxMeshing )
 
SUBDIRS(
  IO
  Filter
  CommandLine
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
