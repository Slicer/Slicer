# Module libraries built with the macros that extensions build theirs with (SlicerMacroBuildModuleMRML,
# SlicerMacroBuildModuleLogic), in Slicer's own build. Slicer's own module libraries are VTK modules,
# so without these, nothing in Slicer's build and tests would use those macros the way an extension
# does. test_slicer_legacy_module_macros.py checks that their classes, derived from Slicer's
# (vtkMRMLNode, vtkSlicerModuleLogic), are wrapped for Python with their base classes: the wrapping
# finds the hierarchy files of Slicer's libraries, or it makes classes that cannot be instantiated.

set(MODULE_NAME LegacyModuleMacrosTest)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

add_subdirectory(MRML)
add_subdirectory(Logic)
