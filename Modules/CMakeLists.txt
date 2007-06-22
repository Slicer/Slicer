PROJECT( Modules )

SUBDIRS(
  NeuroNav
  RealTimeImaging
  WFEngineModule
  CommandLineModule
  ScriptedModule
  GradientAnisotropicDiffusionFilter
  EMSegment
  SlicerDaemon
  Editor
  QueryAtlas
  Tractography
  #Meshing
#  QdecModule
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
