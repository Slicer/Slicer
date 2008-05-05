project(Modules)

subdirs(
  Volumes
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

if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)

if(USE_PYTHON AND PYTHON_INCLUDE_PATH)
  include_directories(${PYTHON_INCLUDE_PATH})
endif(USE_PYTHON AND PYTHON_INCLUDE_PATH)
