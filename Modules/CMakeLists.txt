project(Modules)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

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
#  GradientAnisotropicDiffusionFilter <- Moved into Attic
  Meshing
  Python
# PythonGADScriptedModule <- Moved into Attic
#  QueryAtlas <- Moved into Attic
  FetchMI
  SlicerWelcome
  DiffusionWelcome
  RegistrationWelcome
  SegmentationWelcome
#  ScriptedModule <- Moved into Attic
#  ScriptedModuleExample <- Moved into Attic
  SlicerDaemon
  Statistics
  Tractography
# TumorGrowth
  Volumes
  ChangeTracker # AF: refactoring TumorGrowth...
  VolumeRendering
#  VolumeRendering1 <- merged into VolumeRendering <- Moved into Attic
# WFEngineModule  <- can't work with 2.6 and Win32??
  ModelMirror
  ModelTransform
  # EMSegment # EMSegment is build from the top-level cmake
  # DistanceTransformModel <- culled from slicer 3.6
  ClipModel
#  Measurements <- Moved into Attic
  ModelIntoLabelVolume
# ExtractSubvolume <- replaced by ExtractSubvolumeROI
  FastMarchingSegmentation
  FourDImage
#  FourDAnalysis
#  ExtractSubvolumeROI <- renamed to CropVolume <- Moved into Attic
  CropVolume
  CollectFiducials
  PETCTFusion
  IGTToolSelector
  )

if(Slicer_USE_OPENIGTLINK)
  SET(modules
    ${modules}
    OpenIGTLinkIF
    NeuroNav
    ProstateNav
    BrainlabModule
    )
endif(Slicer_USE_OPENIGTLINK)

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
## TRACTOGRAPHY_DEBUG
## COMMANDLINE_DEBUG
## DEAMON_DEBUG

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)

