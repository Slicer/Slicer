
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

# With the Makefiles generator, the Python wrapping hierarchy of a module can
# only depend on the hierarchy target of another module if that target already
# exists when the module is configured (see vtkWrapHierarchy.cmake in
# vtkAddon). For modules that depend on a module configured later in the list
# above, the dependency is silently dropped and parallel builds can fail with
# "vtkWrapHierarchy: couldn't open file ...Hierarchy.txt". Restore the known
# forward dependencies here, where all hierarchy targets exist.
foreach(_hierarchy_dependency IN ITEMS
    "vtkSlicerTransformsModuleLogicHierarchy;vtkSlicerMarkupsModuleMRMLHierarchy"
    "vtkSlicerSegmentationsModuleLogicHierarchy;vtkSlicerMarkupsModuleMRMLHierarchy"
    )
  list(GET _hierarchy_dependency 0 _hierarchy_target)
  list(GET _hierarchy_dependency 1 _hierarchy_dependee)
  if(TARGET ${_hierarchy_target} AND TARGET ${_hierarchy_dependee})
    add_dependencies(${_hierarchy_target} ${_hierarchy_dependee})
  endif()
endforeach()
