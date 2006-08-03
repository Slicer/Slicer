PROJECT( Modules )

SUBDIRS(
  BaseClasses
  CommandLineModule
  ScriptedModule
  GradientAnisotropicDiffusionFilter
  SlicerDaemon
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
