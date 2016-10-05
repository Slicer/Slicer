
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

set(qtmodules
  AtlasCreator
  Cameras
  Units
  Colors
  SubjectHierarchy
  Transforms
  Annotations
  Data
  DoubleArrays
  Markups
  #Measurements
  Models
  #FiberBundles
  SceneViews
  Segmentations
  SlicerWelcome
  Tables
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

foreach(module ${qtmodules})
  list(FIND Slicer_QTLOADABLEMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach(module)

# External modules have been moved to Modules/Remote/CMakeLists.txt
