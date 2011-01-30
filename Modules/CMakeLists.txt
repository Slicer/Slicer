project(Modules)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer_SOURCE_DIR)

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

