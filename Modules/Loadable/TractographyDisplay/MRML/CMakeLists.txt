project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

if(MRML_USE_vtkTeem)
  list(APPEND ${KIT}_INCLUDE_DIRECTORIES ${vtkTeem_INCLUDE_DIRS})
endif()

set(${KIT}_SRCS
  vtkMRMLFiberBundleDisplayNode.cxx
  vtkMRMLFiberBundleDisplayNode.h
  )
if(MRML_USE_vtkTeem)
  list(APPEND ${KIT}_SRCS
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

#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#if(${VTK_VERSION_MAJOR} GREATER 5)
#  set(VTK_LIBRARIES
#    vtkFiltersExtraction
#    )
#endif()

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  ${VTK_LIBRARIES}
   vtkSlicerAnnotationsModuleMRML
  )
if(MRML_USE_vtkTeem)
  list(APPEND ${KIT}_TARGET_LIBRARIES vtkTeem)
endif()

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
