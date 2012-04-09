cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
# Module name
#   - Usually identique to the directory name)
#   - You shouldn't call 'project(...)'. See SlicerConfig.cmake for more details.
#-----------------------------------------------------------------------------
set(EXTENSION_NAME EditorExtensionTemplate)

#-----------------------------------------------------------------------------
# Prerequisites
#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Slicer4:Developers:Projects:QtSlicer/Tutorials/ExtensionWriting")
  set(EXTENSION_CATEGORY "Examples")
  set(EXTENSION_STATUS "Beta")
  set(EXTENSION_DESCRIPTION "This is an example of scripted loadable editor effect built as an extension")
  set(EXTENSION_DEPENDS NA) # Specified as a space separated list or 'NA' if any

  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})

  set(EXTENSION_LICENSE_FILE ${Slicer_LICENSE_FILE})
  set(EXTENSION_README_FILE ${Slicer_README_FILE})

  include(SlicerEnableExtensionTesting)

endif()

#-----------------------------------------------------------------------------
# Sources
#-----------------------------------------------------------------------------
set(KIT_PYTHON_SCRIPTS
  EditorExtensionTemplate.py
  )

set(KIT_PYTHON_RESOURCES
  EditorExtensionTemplate.png
  )

#-----------------------------------------------------------------------------
# Build
#-----------------------------------------------------------------------------
ctkMacroCompilePythonScript(
  TARGET_NAME EditorExtensionTemplate
  SCRIPTS "${KIT_PYTHON_SCRIPTS}"
  RESOURCES "${KIT_PYTHON_RESOURCES}"
  DESTINATION_DIR ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  INSTALL_DIR ${Slicer_INSTALL_ROOT}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
# Package and generate extension description file '<EXTENSION_NAME>.s4ext'
#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
