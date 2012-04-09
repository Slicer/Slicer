cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME EditorExtensionTemplate) # Do not use 'project()' - See SlicerConfig.cmake

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Snapshot/Developers/Tutorials/ExtensionWriting")
  set(EXTENSION_CATEGORY "Examples")
  set(EXTENSION_STATUS "Beta")
  set(EXTENSION_DESCRIPTION "This is an example of scripted loadable editor effect built as an extension")
  set(EXTENSION_DEPENDS NA) # Specified as a space separated list or 'NA' if any

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  include(SlicerEnableExtensionTesting)
endif()

#-----------------------------------------------------------------------------
set(KIT_PYTHON_SCRIPTS
  EditorExtensionTemplate.py
  )

set(KIT_PYTHON_RESOURCES
  EditorExtensionTemplate.png
  )

#-----------------------------------------------------------------------------
ctkMacroCompilePythonScript(
  TARGET_NAME EditorExtensionTemplate
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_ROOT}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
