PROJECT( Tractography )

SUBDIRS(
#  Editor
#  Seeding
  Settings
  Display
  FiducialSeeding
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
