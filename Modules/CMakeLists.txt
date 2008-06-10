project(Modules)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

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
#   - QdecModule
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

subdirs(
   CommandLineModule
   Editor
   GradientAnisotropicDiffusionFilter
   Meshing
   Python
   PythonGADScriptedModule
   QdecModule
   QueryAtlas
   ScriptedModule
   SlicerDaemon
   Statistics
   Tractography
   #TumorGrowth
   VolumeRendering
#  WFEngineModule  <- can't work with 2.6 and Win32??
#  MRAblation <- not working either
   Volumes
   EMSegment
  )

if(Slicer3_USE_NAVITRACK)
  subdirs(
    RealTimeImaging
    )
endif(Slicer3_USE_NAVITRACK)

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
## TRACTOGRAPHY_DEBUG
## QDEC_DEBUG
## COMMANDLINE_DEBUG
## DEAMON_DEBUG

if(Slicer3_BUILD_TESTING)
#  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)
