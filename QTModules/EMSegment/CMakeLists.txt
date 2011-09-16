# --------------------------------------------------------------------------
#
# EM Segmenter for 3D Slicer3 and 3D Slicer4
#
# --------------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# --------------------------------------------------------------------------
# Detect if we build for Slicer3 or Slicer4 and set EM_Slicer4_FOUND accordingly
#
include(CMake/EM_DetectSlicerVersion.cmake)



# --------------------------------------------------------------------------
# CMake Requirements and Policies
#
if(EM_Slicer3_FOUND)
  # include Slicer3 specific file
  include(CMake/EM_Slicer3_CMakeRequirements.cmake)
endif()

if(EM_Slicer4_FOUND)
  # include Slicer4 specific file
  include(CMake/EM_Slicer4_CMakeRequirements.cmake)
endif()



# --------------------------------------------------------------------------
# Find package Slicer and include it
#
if(EM_Slicer3_FOUND)
  # include Slicer3 specific file
  include(CMake/EM_Slicer3_FindPackageSlicer.cmake)
endif()

if(EM_Slicer4_FOUND)
  # include Slicer4 specific file
  include(CMake/EM_Slicer4_FindPackageSlicer.cmake)
endif()



# --------------------------------------------------------------------------
# Define the EMSegment Base Sources (MRML, Algorithms, Logic etc.) 
include(CMake/EM_BaseSources.cmake)



# --------------------------------------------------------------------------
# Install Task-files (.tcl and .mrml files)
include(CMake/EM_InstallTasks.cmake)



# --------------------------------------------------------------------------
# Slicer3 specific: Include Slicer3 Python
if(EM_Slicer3_FOUND)
  include(CMake/EM_Slicer3_IncludeSlicerPython.cmake)
endif()



# --------------------------------------------------------------------------
# Slicer3 specific: Generate EMSegment Configure File
if(EM_Slicer3_FOUND)
  include(CMake/EM_Slicer3_GenerateConfigureFile.cmake)
endif()



# --------------------------------------------------------------------------
# Install Test-Data and include the Testing/ sub-directory
include(CMake/EM_Testing.cmake)



# --------------------------------------------------------------------------
# Include the commandline application
include(CMake/EM_CommandLineApplication.cmake)



# --------------------------------------------------------------------------
# Add the GUI classes depending on the Slicer version
#
if(EM_Slicer3_FOUND)
  include(CMake/EM_Slicer3_GraphicalUserInterface.cmake)
endif()

if(EM_Slicer4_FOUND)
  include(CMake/EM_Slicer4_GraphicalUserInterface.cmake)
endif()



# --------------------------------------------------------------------------
# Slicer4 specific: Generate Extension Files
if(EM_Slicer4_FOUND)
  include(CMake/EM_Slicer4_GenerateExtensionFile.cmake)
endif()  



