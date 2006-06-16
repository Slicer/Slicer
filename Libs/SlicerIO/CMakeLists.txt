PROJECT(SlicerIO)

# look for the Tcl library
IF (TCL_LIBRARY)
ELSE(TCL_LIBRARY)
    MESSAGE(FATAL_ERROR "Could not find tcl")
ENDIF (TCL_LIBRARY)

ADD_LIBRARY(slicerio utilities.c slicerio.c)

ADD_EXECUTABLE(testSlicerIO testSlicerIO.c)
TARGET_LINK_LIBRARIES(testSlicerIO slicerio ${TCL_LIBRARY})

ADD_EXECUTABLE(scat scat.c)
TARGET_LINK_LIBRARIES(scat slicerio ${TCL_LIBRARY})

OPTION (MY_DEBUG
    "Building debug code"
    ON)
IF(MY_DEBUG)
    SET_SOURCE_FILES_PROPERTIES(
        utilities.c slicerio.c testSlicerIO.c scat.c
        COMPILE_FLAGS -g)
ENDIF(MY_DEBUG)
