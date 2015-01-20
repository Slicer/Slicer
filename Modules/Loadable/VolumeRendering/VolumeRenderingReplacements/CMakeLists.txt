project(VolumeRenderingReplacements)
#-----------------------------------------------------------------------------
#
# Third-Party
#
find_package(OpenGL REQUIRED)

set(module_replacements_name "${PROJECT_NAME}")

# The header '${module_replacements_name}Export.h' will be automatically configured.
set(module_replacements_export_directive "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_REPLACEMENTS_EXPORT")

set(module_replacements_include_directories
  #${qt_module_include_directories}
  )

set(module_replacements_SRCS
  # GPU Ray Cast stuff
  vtkSlicerGPUVolumeMapper.cxx
  vtkSlicerGPUVolumeMapper.h
  vtkSlicerGPURayCastMultiVolumeMapper.cxx
  vtkSlicerGPURayCastMultiVolumeMapper.h
  vtkSlicerGPUMultiVolumeMapper.cxx
  vtkSlicerGPUMultiVolumeMapper.h
  # Ray Cast stuff
  vtkSlicerFixedPointRayCastImage.cxx
  vtkSlicerFixedPointRayCastImage.h
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeGOShadeHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeHelper.h
  vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.cxx
  vtkSlicerFixedPointVolumeRayCastCompositeShadeHelper.h
  vtkSlicerFixedPointVolumeRayCastHelper.cxx
  vtkSlicerFixedPointVolumeRayCastHelper.h
  vtkSlicerFixedPointVolumeRayCastMapper.cxx
  vtkSlicerFixedPointVolumeRayCastMapper.h
  vtkSlicerFixedPointVolumeRayCastMIPHelper.cxx
  vtkSlicerFixedPointVolumeRayCastMIPHelper.h
  vtkSlicerRayCastImageDisplayHelper.cxx
  vtkSlicerRayCastImageDisplayHelper.h
  # OpenGL
  vtkSlicerOpenGLRayCastImageDisplayHelper.h
  vtkSlicerOpenGLRayCastImageDisplayHelper.cxx
  )

if (${VTK_VERSION_MAJOR} GREATER 5)
  # Now for the object factory.
  set(opengl_overrides
    RayCastImageDisplayHelper
    )

  macro(vtk_add_override base override)
    list(APPEND vtk_module_overrides ${base})
    set(vtk_module_${base}_override ${override})
  endmacro()

  foreach(_override ${opengl_overrides})
    vtk_add_override(vtkSlicer${_override} vtkSlicerOpenGL${_override})
  endforeach()

  # Now we iterate and create that class file...
  set(vtk-module ${PROJECT_NAME})
  set(VTK-MODULE Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_REPLACEMENTS)
  foreach(_class ${vtk_module_overrides})
    set(_override ${vtk_module_${_class}_override})
    set(_vtk_override_includes "${_vtk_override_includes}
#include \"${_override}.h\"")
    set(_vtk_override_creates "${_vtk_override_creates}
  VTK_CREATE_CREATE_FUNCTION(${_override})")
    set(_vtk_override_do "${_vtk_override_do}
    this->RegisterOverride(\"${_class}\",
                           \"${_override}\",
                           \"Override for ${vtk-module} module\", 1,
                           vtkObjectFactoryCreate${_override});")
  endforeach()

  # Now lets create the object factory classes
  file(READ ${VTK_CMAKE_DIR}/vtkObjectFactory.h.in file_content)
  string(REGEX REPLACE "Module.h" "Export.h" modified_file_content "${file_content}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/vtkObjectFactory.h.in "${modified_file_content}")
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/vtkObjectFactory.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.h)
  configure_file(${VTK_CMAKE_DIR}/vtkObjectFactory.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.cxx)
  list(APPEND Module_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${vtk-module}ObjectFactory.cxx)
  set_source_files_properties(
    ${vtk-module}ObjectFactory.cxx
    WRAP_EXCLUDE
    )
  list(APPEND module_replacements_SRCS ${vtk-module}ObjectFactory.cxx)
else()
  list(APPEND module_replacements_SRCS
    vtkSlicerVolumeRenderingFactory.cxx
    vtkSlicerVolumeRenderingFactory.h
    )
endif()

set(module_replacements_target_libraries
  ${VTK_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${module_replacements_name}
  EXPORT_DIRECTIVE ${module_replacements_export_directive}
  INCLUDE_DIRECTORIES ${module_replacements_include_directories}
  SRCS ${module_replacements_SRCS}
  TARGET_LIBRARIES ${module_replacements_target_libraries}
  )
