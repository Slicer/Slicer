#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.2)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------
if(POLICY CMP0016)
  CMAKE_POLICY(SET CMP0016 NEW)
endif()
if(POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
endif()

# Module name (usually identique to the directory name)
SET(EXTENSION_NAME LoadableExtensionTemplate)

IF(NOT Slicer_SOURCE_DIR)

  SET(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
  SET(EXTENSION_CATEGORY "Examples")
  SET(EXTENSION_STATUS "Beta")
  SET(EXTENSION_DESCRIPTION "This is an example of Qt loadable module built as an extension")

  FIND_PACKAGE(Slicer REQUIRED)
  
  SET(ADDITIONAL_C_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} C Flags")
  SET(ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional ${EXTENSION_NAME} CXX Flags")
  
  INCLUDE(${Slicer_USE_FILE})
  
  SET(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
  SET(EXTENSION_README_FILE ${Slicer_README_FILE})

  INCLUDE(CTest)

ENDIF()


# Add subdirectories
ADD_SUBDIRECTORY(Logic)

SET(qt_module_export_directive "Q_SLICER_QTMODULES_LOADABLEEXTENSIONTEMPLATE_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

# Source files
SET(qt_module_SRCS
  qSlicerLoadableExtensionTemplateModule.cxx
  qSlicerLoadableExtensionTemplateModule.h
  qSlicerLoadableExtensionTemplateModuleWidget.cxx
  qSlicerLoadableExtensionTemplateModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  qSlicerLoadableExtensionTemplateModule.h
  qSlicerLoadableExtensionTemplateModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerLoadableExtensionTemplateModule.ui
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerLoadableExtensionTemplateModuleLogic
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerLoadableExtensionTemplateModule.qrc
)

slicerMacroBuildQtModule(
  NAME ${EXTENSION_NAME}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )


if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

IF(NOT Slicer_SOURCE_DIR)
  INCLUDE(SlicerFunctionGenerateExtensionDescription)
  slicerFunctionGenerateExtensionDescription(
    EXTENSION_NAME ${EXTENSION_NAME}
    EXTENSION_CATEGORY ${EXTENSION_CATEGORY} 
    EXTENSION_STATUS ${EXTENSION_STATUS}
    EXTENSION_HOMEPAGE ${EXTENSION_HOMEPAGE}
    EXTENSION_DESCRIPTION ${EXTENSION_DESCRIPTION}
    DESTINATION_DIR ${CMAKE_CURRENT_BINARY_DIR}
    SLICER_WC_REVISION ${Slicer_WC_REVISION}
    SLICER_WC_ROOT ${Slicer_WC_ROOT}
    )
  INCLUDE(${Slicer_EXTENSION_CPACK})
ENDIF()
