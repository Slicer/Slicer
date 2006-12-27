PROJECT( Modules )

SUBDIRS(
  BaseClasses
  CommandLineModule
  ScriptedModule
  GradientAnisotropicDiffusionFilter
  SlicerDaemon
  Editor
  QueryAtlas
  Tractography
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
