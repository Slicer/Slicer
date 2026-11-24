project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_SRCS
  vtkSlicerLegacyModuleMacrosTestLogic.cxx
  vtkSlicerLegacyModuleMacrosTestLogic.h
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  )

SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  NO_INSTALL
  )
