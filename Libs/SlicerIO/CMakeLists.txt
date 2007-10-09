PROJECT(SlicerIO)

# look for the Tcl library
IF (TCL_LIBRARY)
ELSE(TCL_LIBRARY)
    MESSAGE(FATAL_ERROR "Could not find tcl")
ENDIF (TCL_LIBRARY)

ADD_LIBRARY(slicerio STATIC utilities.c slicerio.c)

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS slicerio
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES (slicerio ${TCL_LIBRARY})

ADD_EXECUTABLE(testSlicerIO testSlicerIO.c)
TARGET_LINK_LIBRARIES(testSlicerIO slicerio ${TCL_LIBRARY})

ADD_EXECUTABLE(scat scat.c)
TARGET_LINK_LIBRARIES(scat slicerio ${TCL_LIBRARY})

# This seems to be required for a static build against a static Tcl/Tk

IF(NOT WIN32 AND VTK_TCL_TK_STATIC)
  TARGET_LINK_LIBRARIES(scat m dl)
  TARGET_LINK_LIBRARIES(testSlicerIO m dl)
ENDIF(NOT WIN32 AND VTK_TCL_TK_STATIC)

