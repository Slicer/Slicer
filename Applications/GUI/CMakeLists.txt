PROJECT( GUI )

SET (SRC
    Slicer3.cxx
)

INCLUDE_DIRECTORIES (
    ${Slicer3_SOURCE_DIR}/Base/Logic
    ${Slicer3_SOURCE_DIR}/Base/GUI
)

ADD_EXECUTABLE (Slicer3 ${SRC})

IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
