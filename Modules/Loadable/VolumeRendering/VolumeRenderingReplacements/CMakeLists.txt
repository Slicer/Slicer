project(VolumeRenderingReplacements)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

#
# Third-Party
#
find_package(OpenGL REQUIRED)

set(module_replacements_name "${PROJECT_NAME}")

# The header '${module_replacements_name}Export.h' will be automatically configured.
set(module_replacements_export_directive "Q_SLICER_QTMODULES_VOLUMERENDERING_REPLACEMENTS_EXPORT")

# Additional directories to include
set(module_replacements_include_directories
  ${qt_module_include_directories}
  )

# Source files
set(module_replacements_SRCS
  vtkSlicerVolumeTextureMapper3D.cxx
  vtkSlicerOpenGLVolumeTextureMapper3D.cxx
  vtkSlicerVolumeRenderingFactory.cxx
  vtkSlicerGPURayCastVolumeMapper.cxx
  vtkSlicerGPUVolumeMapper.cxx
  vtkSlicerGPURayCastMultiVolumeMapper.cxx
  vtkSlicerGPUMultiVolumeMapper.cxx
  vtkSlicerFixedPointRayCastImage.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastHelper.cxx
  vtkSlicerFixedPointVolumeRayCastMapper.cxx
  vtkSlicerFixedPointVolumeRayCastMIPHelper.cxx
  vtkSlicerRayCastImageDisplayHelper.cxx
  vtkSlicerOpenGLRayCastImageDisplayHelper.cxx
  #vtkSlicerBoxWidget.cxx
  #vtkSlicerVolumePropertyWidget.cxx
  #vtkSlicerBoxWidget2.cxx
  #vtkSlicerBoxRepresentation.cxx
  # Headers
  vtkSlicerVolumeTextureMapper3D.h
  vtkSlicerOpenGLVolumeTextureMapper3D.h
  vtkSlicerVolumeRenderingFactory.h
  #GPU Ray Cast stuff
  vtkSlicerGPURayCastVolumeMapper.h
  vtkSlicerGPUVolumeMapper.h
  vtkSlicerGPURayCastMultiVolumeMapper.h
  vtkSlicerGPUMultiVolumeMapper.h
  #Ray Cast stuff
  vtkSlicerFixedPointRayCastImage.h
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.h
  vtkSlicerFixedPointVolumeRayCastHelper.h
  vtkSlicerFixedPointVolumeRayCastMapper.h
  vtkSlicerFixedPointVolumeRayCastMIPHelper.h
  vtkSlicerRayCastImageDisplayHelper.h
  vtkSlicerOpenGLRayCastImageDisplayHelper.h
  #vtkSlicerBoxWidget.h
  #vtkSlicerVolumePropertyWidget.h
  #vtkSlicerBoxWidget2.h
  #vtkSlicerBoxRepresentation.h

  )

# Additional Target libraries
set(module_replacements_target_libraries
  ${VTK_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  )

SlicerMacroBuildModuleLogic(
  NAME ${module_replacements_name}
  EXPORT_DIRECTIVE ${module_replacements_export_directive}
  INCLUDE_DIRECTORIES ${module_replacements_include_directories}
  SRCS ${module_replacements_SRCS}
  TARGET_LIBRARIES ${module_replacements_target_libraries}
  )
