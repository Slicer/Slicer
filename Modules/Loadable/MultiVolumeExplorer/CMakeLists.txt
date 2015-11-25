
cmake_minimum_required(VERSION 2.8.4)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0017)
  cmake_policy(SET CMP0017 OLD)
endif()

#-----------------------------------------------------------------------------
set(MODULE_NAME MultiVolumeExplorer)
set(MODULE_TITLE ${MODULE_NAME})

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)
add_subdirectory(Py)

#-----------------------------------------------------------------------------
set(qt_module_export_directive "Q_SLICER_QTMODULES_MULTIVOLUMEEXPLORER_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  )

set(qt_module_SRCS
  qSlicerMultiVolumeExplorerModule.cxx
  qSlicerMultiVolumeExplorerModule.h
  )

set(qt_module_MOC_SRCS
  qSlicerMultiVolumeExplorerModule.h
  )

set(qt_module_UI_SRCS
)

set(qt_module_target_libraries
  vtkSlicerMultiVolumeExplorerModuleMRML
  vtkSlicerMultiVolumeExplorerModuleLogic
  )

set(qt_module_resources
  Resources/qSlicerMultiVolumeExplorerModule.qrc
)

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
