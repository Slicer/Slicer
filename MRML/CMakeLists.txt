project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
)

set(${KIT}_SRCS
  vtkMRMLLayerDMNodeReferenceObserver.cxx
  vtkMRMLLayerDMNodeReferenceObserver.h
  vtkMRMLLayerDMObjectEventObserver.cxx
  vtkMRMLLayerDMObjectEventObserver.h
  vtkMRMLLayerDMSelectionObserver.cxx
  vtkMRMLLayerDMSelectionObserver.h
  vtkMRMLLayerDMWidgetEventTranslationNode.cxx
  vtkMRMLLayerDMWidgetEventTranslationNode.h
)


if (VTK_WRAP_PYTHON)
  list(APPEND ${KIT}_SRCS
    vtkMRMLLayerDMObjectEventObserverScripted.cxx
    vtkMRMLLayerDMObjectEventObserverScripted.h
    vtkMRMLLayerDMPythonUtil.cxx
    vtkMRMLLayerDMPythonUtil.h
  )
endif ()

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
)

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
)

target_compile_features(${KIT} PUBLIC cxx_std_17)

target_include_directories(
  ${KIT}
  PUBLIC
  ${${KIT}_SOURCE_DIR}
  ${${KIT}_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
)
