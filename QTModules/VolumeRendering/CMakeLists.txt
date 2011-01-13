
# Add subdirectories
ADD_SUBDIRECTORY(Logic)

# Module name (usually identique to the directory name)
SET(qt_module_name "VolumeRendering")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

# Source files
SET(qt_module_SRCS
  qSlicerVolumeRenderingModule.cxx
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.cxx
  qSlicerVolumeRenderingModuleWidget.h

  MRML/vtkMRMLVolumeRenderingParametersNode.cxx
  MRML/vtkMRMLVolumeRenderingScenarioNode.cxx
  Logic/vtkSlicerVolumeRenderingLogic.cxx
  Logic/vtkMRMLVolumeRenderingDisplayableManager.cxx
  VolumeRenderingReplacements/vtkSlicerVolumeTextureMapper3D.cxx
  VolumeRenderingReplacements/vtkSlicerOpenGLVolumeTextureMapper3D.cxx
  VolumeRenderingReplacements/vtkSlicerVolumeRenderingFactory.cxx
  #GPU Ray Cast stuff
  VolumeRenderingReplacements/vtkSlicerGPURayCastVolumeMapper.cxx
  VolumeRenderingReplacements/vtkSlicerGPUVolumeMapper.cxx
  VolumeRenderingReplacements/vtkSlicerGPURayCastMultiVolumeMapper.cxx
  VolumeRenderingReplacements/vtkSlicerGPUMultiVolumeMapper.cxx
  #Ray Cast stuff
  VolumeRenderingReplacements/vtkSlicerFixedPointRayCastImage.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeHelper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastHelper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastMapper.cxx
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastMIPHelper.cxx
  VolumeRenderingReplacements/vtkSlicerRayCastImageDisplayHelper.cxx
  VolumeRenderingReplacements/vtkSlicerOpenGLRayCastImageDisplayHelper.cxx
  #GUI
  #VolumeRenderingReplacements/vtkSlicerBoxWidget.cxx
  #VolumeRenderingReplacements/vtkSlicerVolumePropertyWidget.cxx
  #VolumeRenderingReplacements/vtkSlicerBoxWidget2.cxx
  #VolumeRenderingReplacements/vtkSlicerBoxRepresentation.cxx

  MRML/vtkMRMLVolumeRenderingParametersNode.h
  MRML/vtkMRMLVolumeRenderingScenarioNode.h
  Logic/vtkSlicerVolumeRenderingLogic.h
  Logic/vtkMRMLVolumeRenderingDisplayableManager.h
  VolumeRenderingReplacements/vtkSlicerVolumeTextureMapper3D.h
  VolumeRenderingReplacements/vtkSlicerOpenGLVolumeTextureMapper3D.h
  VolumeRenderingReplacements/vtkSlicerVolumeRenderingFactory.h
  #GPU Ray Cast stuff
  VolumeRenderingReplacements/vtkSlicerGPURayCastVolumeMapper.h
  VolumeRenderingReplacements/vtkSlicerGPUVolumeMapper.h
  VolumeRenderingReplacements/vtkSlicerGPURayCastMultiVolumeMapper.h
  VolumeRenderingReplacements/vtkSlicerGPUMultiVolumeMapper.h
  #Ray Cast stuff
  VolumeRenderingReplacements/vtkSlicerFixedPointRayCastImage.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeHelper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastHelper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastMapper.h
  VolumeRenderingReplacements/vtkSlicerFixedPointVolumeRayCastMIPHelper.h
  VolumeRenderingReplacements/vtkSlicerRayCastImageDisplayHelper.h
  VolumeRenderingReplacements/vtkSlicerOpenGLRayCastImageDisplayHelper.h
  #GUI
  #VolumeRenderingReplacements/vtkSlicerBoxWidget.h
  #VolumeRenderingReplacements/vtkSlicerVolumePropertyWidget.h
  #VolumeRenderingReplacements/vtkSlicerBoxWidget2.h
  #VolumeRenderingReplacements/vtkSlicerBoxRepresentation.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerVolumeRenderingModule.h
  qSlicerVolumeRenderingModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerVolumeRenderingModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkVolumeRendering
#  vtkSlicerVolumeRenderingModuleLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerVolumeRenderingModule.qrc
)

slicerMacroBuildQtModule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif(BUILD_TESTING)
