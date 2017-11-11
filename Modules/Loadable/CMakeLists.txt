
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildLoadableModule.cmake)

set(qtmodules
  AtlasCreator
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
  #Measurements
  Models
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
  set(_build_module 1)
  # Is module expicitly disabled ?
  if(Slicer_QTLOADABLEMODULES_DISABLED)
    list(FIND Slicer_QTLOADABLEMODULES_DISABLED ${module} _module_disabled)
    if(${_module_disabled} GREATER -1)
      set(_build_module 0)
    endif()
  endif()
  # Is module explicitly enabled ?
  if(Slicer_QTLOADABLEMODULES_ENABLED)
    list(FIND Slicer_QTLOADABLEMODULES_ENABLED ${module} _module_enabled)
    if(NOT ${_module_enabled} GREATER -1)
      set(_build_module 0)
    endif()
  endif()
  if(_build_module)
    add_subdirectory(${module})
  endif()
endforeach(module)

# External modules have been moved to Modules/Remote/CMakeLists.txt
