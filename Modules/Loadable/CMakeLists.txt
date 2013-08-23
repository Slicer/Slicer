
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

set(qtmodules
  AtlasCreator
  Cameras
  Units
  Colors
  Transforms
  Annotations
  Data
  DoubleArrays
  FetchMI
  Markups
  #Measurements
  Models
  ModelMirror
  ModelTransform
  #FiberBundles
  SceneViews
  SlicerWelcome
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
if(Slicer_BUILD_DIFFUSION_SUPPORT)
  list(APPEND qtmodules
    TractographyDisplay
    TractographyInteractiveSeeding # tractography depends of Volumes
    )
endif()

foreach(module ${qtmodules})
  list(FIND Slicer_QTLOADABLEMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach(module)

# External modules have been moved to Modules/Remote/CMakeLists.txt
