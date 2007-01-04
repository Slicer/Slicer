PROJECT(SlicerIO)

# look for the Tcl library
IF (TCL_LIBRARY)
ELSE(TCL_LIBRARY)
    MESSAGE(FATAL_ERROR "Could not find tcl")
ENDIF (TCL_LIBRARY)

ADD_LIBRARY(slicerio STATIC utilities.c slicerio.c)

IF(BUILD_SHARED_LIBS)
  INSTALL_TARGETS(${SLICER3_LIB_INSTALL_DIR} slicerio)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES (slicerio ${TCL_LIBRARY})

ADD_EXECUTABLE(testSlicerIO testSlicerIO.c)
TARGET_LINK_LIBRARIES(testSlicerIO slicerio ${TCL_LIBRARY})

ADD_EXECUTABLE(scat scat.c)
TARGET_LINK_LIBRARIES(scat slicerio ${TCL_LIBRARY})

# This seems to be required for a static build against a static Tcl/Tk

IF(VTK_TCL_TK_STATIC)
  TARGET_LINK_LIBRARIES(scat m dl)
  TARGET_LINK_LIBRARIES(testSlicerIO m dl)
ENDIF(VTK_TCL_TK_STATIC)

