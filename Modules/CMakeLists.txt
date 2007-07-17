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
  Python
  #Meshing
  QdecModule
)


# enable tractography if teem is present
IF( TEEM_FOUND )
  SUBDIRS(
    Tractography
   )
  # set up teem variable for compiling if needed
  ADD_DEFINITIONS(-DUSE_TEEM)
ENDIF( TEEM_FOUND )

IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

IF ( USE_PYTHON AND PYTHON_INCLUDE_PATH )
  INCLUDE_DIRECTORIES (${PYTHON_INCLUDE_PATH})
ENDIF ( USE_PYTHON AND PYTHON_INCLUDE_PATH )
