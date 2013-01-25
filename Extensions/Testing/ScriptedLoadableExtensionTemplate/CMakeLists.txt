cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  set(EXTENSION_NAME ScriptedLoadableExtensionTemplate)
  set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.2/Extensions/ScriptedLoadableExtensionTemplate")
  set(EXTENSION_CATEGORY "Examples")
  set(EXTENSION_CONTRIBUTORS "Jean-Christophe Fillion-Robin (Kitware), Steve Pieper (Isomics)")
  set(EXTENSION_DESCRIPTION "This is an example of extension bundling a scripted loadable module")
  set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/ScriptedLoadableExtensionTemplate/Resources/Icons/ScriptedLoadableExtensionTemplate.png?revision=19437&view=co")
  set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/e/e2/Slicer-r19441-ScriptedLoadableExtensionTemplate-screenshot.png")
endif()

#-----------------------------------------------------------------------------
set(MODULE_NAME ScriptedLoadableExtensionTemplate)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  )

set(MODULE_PYTHON_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS "${MODULE_PYTHON_SCRIPTS}"
  RESOURCES "${MODULE_PYTHON_RESOURCES}"
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # this will regiester the unittest subclass in the main script
  # as a ctest (note that it is also usable as a runtime test).
  slicer_add_python_unittest(SCRIPT ScriptedLoadableExtensionTemplate.py)

  # additional build-time testing goes in here
  add_subdirectory(Testing)
endif()

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
