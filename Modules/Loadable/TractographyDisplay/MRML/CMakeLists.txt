project(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

set(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
set(module_mrml_export_directive "VTK_SLICER_TRACTOGRAPHY_DISPLAY_MODULE_MRML_EXPORT")

# Additional directories to include
set(module_mrml_include_directories
  )

if(MRML_USE_vtkTeem)
  list(APPEND module_mrml_include_directories ${vtkTeem_INCLUDE_DIRS})
endif()

# Source files
set(module_mrml_SRCS
  vtkMRMLFiberBundleDisplayNode.cxx
  vtkMRMLFiberBundleDisplayNode.h
  )
if(MRML_USE_vtkTeem)
  list(APPEND module_mrml_SRCS
    vtkMRMLFiberBundleGlyphDisplayNode.cxx
    vtkMRMLFiberBundleGlyphDisplayNode.h
    vtkMRMLFiberBundleLineDisplayNode.cxx
    vtkMRMLFiberBundleLineDisplayNode.h
    vtkMRMLFiberBundleNode.cxx
    vtkMRMLFiberBundleNode.h
    vtkMRMLFiberBundleStorageNode.cxx
    vtkMRMLFiberBundleStorageNode.h
    vtkMRMLFiberBundleTubeDisplayNode.cxx
    vtkMRMLFiberBundleTubeDisplayNode.h
    )
endif()

# Abstract/pure virtual classes

#set_source_files_properties(
#  vtkMRMLFiberBundleDisplayNode.cxx
#  ABSTRACT
#)

# Additional Target libraries
set(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
   vtkSlicerAnnotationsModuleMRML 
  )
if(MRML_USE_vtkTeem)
  list(APPEND module_mrml_target_libraries vtkTeem)
endif()

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )
