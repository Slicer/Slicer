project(vtkSlicer${qt_module_name}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

set(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
set(module_mrml_export_directive "VTK_SLICER_TRACTOGRAPHY_MODULE_MRML_EXPORT")

option(TRACTOGRAPHY_MRML_USE_Teem "Build Tractography MRML with Teem support." ON)
mark_as_advanced(TRACTOGRAPHY_MRML_USE_Teem)
if(TRACTOGRAPHY_MRML_USE_Teem)
  find_package(Teem REQUIRED)
  include(${Teem_USE_FILE})
  option(TRACTOGRAPHY_MRML_USE_vtkTeem "Build Tractography MRML with vtkTeem support." ON)
  mark_as_advanced(TRACTOGRAPHY_MRML_USE_vtkTeem)
endif()

# Additional directories to include
set(module_mrml_include_directories
  )
message(STATUS "_USE_Teem = ${TRACTOGRAPHY_MRML_USE_Teem}, _USE_vtkTeem = ${TRACTOGRAPHY_MRML_USE_vtkTeem}")

if(TRACTOGRAPHY_MRML_USE_Teem)
  list(APPEND module_mrml_include_directories ${vtkTeem_INCLUDE_DIRS})
endif()

# Source files
set(module_mrml_SRCS
  vtkMRMLFiberBundleDisplayNode.cxx
  vtkMRMLFiberBundleDisplayNode.h
  )
if(TRACTOGRAPHY_MRML_USE_Teem)
  if(TRACTOGRAPHY_MRML_USE_vtkTeem)
    set(module_mrml_SRCS
        ${module_mrml_SRCS}
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
  endif(TRACTOGRAPHY_MRML_USE_vtkTeem)
endif(TRACTOGRAPHY_MRML_USE_Teem)

# Abstract/pure virtual classes

#set_source_files_properties(
#  vtkMRMLFiberBundleDisplayNode.cxx
#  ABSTRACT
#)

# Additional Target libraries
set(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  )
if(TRACTOGRAPHY_MRML_USE_vtkTeem)
  list(APPEND module_mrml_target_libraries vtkTeem)
endif()

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )

MESSAGE(STATUS "******Tractography: module mrml include dirs = \n${Slicer_ModuleMRML_INCLUDE_DIRS}\ntractography mrml include dirs = \n${vtkSlicerTractographyModuleMRML_INCLUDE_DIRS}")
