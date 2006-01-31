PROJECT( Base )

SUBDIRS(
    Logic
    GUI
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
