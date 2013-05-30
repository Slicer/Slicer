cmake_minimum_required(VERSION 2.8.9)

project(EditorExtensionTemplate)

#-----------------------------------------------------------------------------
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/EditorExtensionTemplate")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_CONTRIBUTORS "Steve Pieper (Isomics)")
set(EXTENSION_DESCRIPTION "This is an example of extension bundling one scripted loadable editor effect")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/EditorExtensionTemplate/EditorExtensionTemplate.png?revision=21750&view=co")

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
endif()

#-----------------------------------------------------------------------------
add_subdirectory(EditorEffectTemplate)

#-----------------------------------------------------------------------------
if(NOT Slicer_SOURCE_DIR)
  include(${Slicer_EXTENSION_CPACK})
endif()
