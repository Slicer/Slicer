
include(${Slicer_CMAKE_DIR}/SlicerMacroBuildQtModule.cmake)

set(qtmodules
  AtlasCreator
  Colors
  Transforms
  Annotations
  Data
  FetchMI
  #Measurements
  Models
  ModelMirror
  ModelTransform
  #FiberBundles
  SceneViews
  SlicerWelcome
  TractographyDisplay
  Reformat
  ViewControllers
  Volumes
  TractographyFiducialSeeding # tractography depends of Volumes
  VolumeRendering # Volume Rendering depends of Volumes
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND qtmodules
    CropVolume
    )
endif()
if(BUILD_TESTING)
  list(APPEND qtmodules ModuleTemplate)
endif()

foreach(module ${qtmodules})
  list(FIND Slicer_QTLOADABLEMODULES_DISABLED ${module} _module_disabled)
  if(${_module_disabled} STREQUAL "-1")
    add_subdirectory(${module})
  endif()
endforeach(module)

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_EMSegment)
  # NOTE: EMSegment source code is checkout from Slicer3 repository using
  #       "External_EMSegment.cmake".
  add_subdirectory(
    ${EMSegment_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/EMSegment
    )
endif()

if(Slicer_USE_OpenIGTLink AND Slicer_BUILD_OpenIGTLinkIF)
  # NOTE: OpenIGTLinkIF source code is checkout from NAMICSandBox repository using
  #       "External_OpenIGTLinkIF.cmake".
  add_subdirectory(
    ${OpenIGTLinkIF_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/OpenIGTLinkIF
    )
endif()

if(Slicer_BUILD_MultiVolumeExplorer)
  # NOTE: MultiVolumeExplorer source code is checkout using "External_MultiVolumeExplorer.cmake".
  add_subdirectory(
    ${MultiVolumeExplorer_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/MultiVolumeExplorer
    )
endif()
