PROJECT( Modules )

SUBDIRS(
  BaseClasses
  GradientAnisotropicDiffusionFilter
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
