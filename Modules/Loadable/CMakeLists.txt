
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
  SceneViews
  Markups # Markups depends on SceneViews (for testing) and Annotations
  Transforms
  Data
  Models
  Plots
  Segmentations
  Sequences
  SlicerWelcome
  Tables
  Texts
  Reformat
  ViewControllers
  Volumes
  VolumeRendering # Volume Rendering depends of Volumes
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND qtmodules
    CropVolume
    )
endif()

include(SlicerCheckModuleEnabled)

foreach(module ${qtmodules})
  slicer_is_loadable_builtin_module_enabled(${module} _build_module)
  if(_build_module)
    add_subdirectory(${module})
  endif()
endforeach()
