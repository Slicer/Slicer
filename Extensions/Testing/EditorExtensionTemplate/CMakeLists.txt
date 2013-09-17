cmake_minimum_required(VERSION 2.8.9)

project(EditorExtensionTemplate)

#-----------------------------------------------------------------------------
set(EXTENSION_HOMEPAGE "http://www.slicer.org/slicerWiki/index.php/Documentation/4.3/Extensions/EditorExtensionTemplate")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_CONTRIBUTORS "Steve Pieper (Isomics)")
set(EXTENSION_DESCRIPTION "This is an example of extension bundling one scripted loadable editor effect")
set(EXTENSION_ICONURL "http://viewvc.slicer.org/viewvc.cgi/Slicer4/trunk/Extensions/Testing/EditorExtensionTemplate/EditorExtensionTemplate.png?revision=21750&view=co")
set(EXTENSION_SCREENSHOTURLS "http://wiki.slicer.org/slicerWiki/images/4/42/Slicer-r19441-EditorExtensionTemplate-screenshot.png")

#-----------------------------------------------------------------------------
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
add_subdirectory(EditorEffectTemplate)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
