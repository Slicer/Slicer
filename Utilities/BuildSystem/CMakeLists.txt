cmake_minimum_required(VERSION 2.5)
project(BuildSystem)

# We are not building anything in this pass

mark_as_advanced(
  CMAKE_BACKWARDS_COMPATIBILITY
  CMAKE_BUILD_TYPE
  CMAKE_INSTALL_PREFIX
  EXECUTABLE_OUTPUT_PATH
  LIBRARY_OUTPUT_PATH
  )

# Add our CMake modules directory to the CMAKE_MODULE_PATH

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# SLICER_LOCAL_MODULES_PATH 
#   - where our local Slicer modules can be found
# SLICER_DOWNLOADED_SOURCES_DIRECTORY
#   - where downloaded Slicer modules should go

set(SLICER_LOCAL_MODULES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Modules")
set(SLICER_DOWNLOADED_SOURCES_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Sources")

# Bring the macros located in our CMake subdir

include(SlicerParseModule)
include(SlicerEnableModule)
include(SlicerDownloadModule)

# Parse all local modules sub-dirs found in a dir.
# This goes through every sub-directories in a given directory
# and tries to parse the corresponding module files.
# The list of modules that have been parsed so far can be retrieved
# using slicer_get_parsed_modules.
#
#slicer_parse_modules_directory("${SLICER_LOCAL_MODULES_PATH}")

# Parse a local module

slicer_parse_module_file("${SLICER_LOCAL_MODULES_PATH}/TestModule2/")

# Parse a remote module

slicer_parse_module_url("http://www.na-mic.org/ViewVC/index.cgi/trunk/BuildSystem/Modules/TestModule1/TestModule1.xml?root=NAMICSandBox&view=co")

# Here is how to retrieve module's information

slicer_is_module_unknown(TestModule1 unknown)
if(NOT unknown)
  slicer_get_module_value(TestModule1 Name name)
  slicer_get_module_value(TestModule1 Dependency deps)
  slicer_get_module_value(TestModule1 Author authors)
  message("Name: ${name}\nDependency: ${deps}\nAuthor: ${authors}")
endif(NOT unknown)

# Retrieve the list of all modules parsed so far (locally or remotely), 
# create a USE_ option for each module (based on the module's name), 
# and retrieve the list of modules for which that option is ON (default: OFF).

slicer_get_parsed_modules(modules)
slicer_create_use_modules_options("${modules}")
slicer_get_used_modules("${modules}" used_modules)

# Find the unresolved module dependencies among the current list of module,
# and the resolved modules (i.e. modules with no or completed dependencies)

slicer_get_unresolved_modules_dependencies(
  "${used_modules}" unresolved_dependencies)
slicer_get_resolved_modules(
  "${used_modules}" resolved_modules)

message("All modules:\n${modules}\n\nUsed modules:\n${used_modules}\n\nUnresolved dependencies among used modules:\n${unresolved_dependencies}\n\nResolved used modules:\n${resolved_modules}")

# Create all download/update targets for the resolved modules
# This function can be used to create and connect both download and update
# targets for a list of specific modules.
# The download target name will be created by appending "_download" to the
# module name, unless that target exists already.
# The update target name will be created by appending "_update" to the
# module name, unless that target exists already.
# A unique "download_modules" and "update_modules" target will be created
# that will depend on all modules download and update targets (respectively).

slicer_create_download_and_update_modules_targets(
  "${resolved_modules}" "${SLICER_DOWNLOADED_SOURCES_DIRECTORY}")

