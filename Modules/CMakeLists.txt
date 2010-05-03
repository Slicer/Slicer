project(Modules)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)

 
if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

find_package(GenerateLM REQUIRED)
if(GenerateLM_FOUND)
  include(${GenerateLM_USE_FILE})
endif(GenerateLM_FOUND)

# Dependencies (to be untangled later, for now the whole directory has to
# be built at the same time)
#
# Volumes:
#   - CommandLineModule
#   - Tractography (SlicerTractographyDisplay, SlicerTractographyFiducialSeeding)
# EMSegment:
#   - Volumes
#   - CommandLineModule (at run-time for its CLP Plugins: EMSegmentCommandLine*)
# QueryAtlas:
# MRAblation (if it wasn't broken):
#   - Volumes
# Editor:
#   - ScriptedModule (at run-time for its Tcl package)
# Meshing:
#   - CommandLineModule (at run-time for its CLP Plugins: VoxelMeshModule)
# Python:
#   - CommandLineModule or ScriptedModule (at run-time for its Python Plugins)
# PythonGADScriptedModule:
#   - ScriptedModule (at run-time for its Python scripts)
# SlicerDaemon:
#   - ScriptedModule (at run-time for its Tcl package)

SET(modules
  #AtlasCreator <- culled from slicer 3.6
  CommandLineModule
  Editor
  Endoscopy
  GradientAnisotropicDiffusionFilter
  Meshing
  Python
# PythonGADScriptedModule
  QueryAtlas
  FetchMI
  SlicerWelcome
  ScriptedModule
  ScriptedModuleExample
  SlicerDaemon
  Statistics
  Tractography
# TumorGrowth
  Volumes
  ChangeTracker # AF: refactoring TumorGrowth...
  VolumeRendering
#  VolumeRendering1 <- merged into VolumeRendering
# WFEngineModule  <- can't work with 2.6 and Win32??
# MRAblation <- not working either
  ModelMirror
  ModelTransform
  EMSegment
  # DistanceTransformModel <- culled from slicer 3.6
  ClipModel
  Measurements
  ModelIntoLabelVolume
# ExtractSubvolume <- replaced by ExtractSubvolumeROI
  FastMarchingSegmentation
  FourDImage
#  FourDAnalysis
#  ExtractSubvolumeROI <- renamed to CropVolume
  CropVolume
  CollectFiducials
  PETCTFusion
  IGTToolSelector
  )

if(Slicer3_USE_OPENIGTLINK)
  SET(modules
    ${modules}
    OpenIGTLinkIF
    NeuroNav
    ProstateNav
    # IGTNavigation
    # OpenCV
    )
endif(Slicer3_USE_OPENIGTLINK)

FOREACH(module ${modules})
  ADD_SUBDIRECTORY(${module})
ENDFOREACH(module)

## SLICES_DEBUG
## GAD_DEBUG
## MODELS_DEBUG
## VOLUMES_DEBUG
## QUERYATLAS_DEBUG
## FETCHMI_DEBUG
## WELCOME_DEBUG
## COLORS_DEBUG
## FIDUCIALS_DEBUG
## CAMERA_DEBUG
## EMSEG_DEBUG
## REALTIMEIMAGING_DEBUG
## MRABLATION_DEBUG
## TRACTOGRAPHY_DEBUG
## COMMANDLINE_DEBUG
## DEAMON_DEBUG

if(Slicer3_BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(Slicer3_BUILD_TESTING)

