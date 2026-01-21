project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerLayerDMVersion.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerLayerDMVersion.h
  @ONLY
)

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
)

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
)

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
)

target_include_directories(
  ${KIT}
  PUBLIC
  ${${KIT}_SOURCE_DIR}
  ${${KIT}_BINARY_DIR}
  ${Slicer_ModuleLogic_INCLUDE_DIRS}
)
