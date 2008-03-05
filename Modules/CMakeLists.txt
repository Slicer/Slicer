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
  Python
  PythonGADScriptedModule
  Meshing
  QdecModule
  QueryAtlas
  Tractography
  VolumeRendering
  Statistics
  Volumes
)

## SLICES_DEBUG
## GAD_DEBUG
## MODELS_DEBUG
## VOLUMES_DEBUG
## QUERYATLAS_DEBUG
## COLORS_DEBUG
## FIDUCIALS_DEBUG
## CAMERA_DEBUG
## EMSEG_DEBUG
## REALTIMEIMAGING_DEBUG
## MRABLATION_DEBUG
## NEURONAV_DEBUG
## TRACTOGRAPHY_DEBUG
## QDEC_DEBUG
## COMMANDLINE_DEBUG
## DEAMON_DEBUG



IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)

IF ( USE_PYTHON AND PYTHON_INCLUDE_PATH )
  INCLUDE_DIRECTORIES (${PYTHON_INCLUDE_PATH})
ENDIF ( USE_PYTHON AND PYTHON_INCLUDE_PATH )
