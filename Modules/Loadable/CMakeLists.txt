
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildLoadableModule.cmake)

# "A depends on B" comment in the module list means that
# module B must be in the module list before module A
# (so that include directories provided by B are available
# when configuring module A).

set(qtmodules
  Cameras
  Units
  Terminologies
  SubjectHierarchy
  Colors # Colors depends on SubjectHierarchy
  Annotations
  Transforms
  GeneralizedReformat
  Data
  Models
  Plots
  Segmentations
  Sequences
  SceneViews # Scene views depends on Sequences
  Markups # Markups depends on SceneViews (for testing) and Annotations
  SlicerWelcome
  Tables
  Texts
  Reformat
  ViewControllers
  Volumes
  VolumeRendering # Volume Rendering depends of Volumes
  CropVolume # CropVolume depends of Volumes
  )

include(SlicerCheckModuleEnabled)

foreach(module ${qtmodules})
  slicer_is_loadable_builtin_module_enabled(${module} _build_module)
  if(_build_module)
    add_subdirectory(${module})
  endif()
endforeach()
