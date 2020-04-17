
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildLoadableModule.cmake)

set(qtmodules
  Cameras
  Units
  Terminologies
  Colors
  SubjectHierarchy
  Annotations
  Markups
  Transforms
  Data
  DoubleArrays
  Models
  Plots
  SceneViews
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
