#-----------------------------------------------------------------------------
#
#  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
#  Reserved.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Program:   3D Slicer
#  Module:    $RCSfile: CMakeLists.txt,v $
#  Date:      $Date: 2009/06/01 17:56:51 $
#  Version:   $Revision: 1.0 $
#  Author:    $Sylvain Jaume (MIT)$
#  URL:       $http://www.slicer.org/slicerWiki/index.php/Modules:MRIBiasFieldCorrection-Documentation-3.5$
#
#-----------------------------------------------------------------------------

project(MRIBiasFieldCorrection)

#-----------------------------------------------------------------------------
# Disable MSVC 8 warnings

if (WIN32)

  option (DISABLE_MSVC8_DEPRECATED_WARNINGS
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced (FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)

  if (DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE)
  endif (DISABLE_MSVC8_DEPRECATED_WARNINGS)

endif (WIN32)


#-----------------------------------------------------------------------------
# Source files

set (${PROJECT_NAME}_SOURCE ${PROJECT_NAME}.cxx)

generateclp (${PROJECT_NAME}_SOURCE ${PROJECT_NAME}.xml
  ${Slicer3_SOURCE_DIR}/Resources/NAMICLogo.h)


#-----------------------------------------------------------------------------
# Build library

add_library (MRIBiasFieldCorrectionModule SHARED
  ${${PROJECT_NAME}_SOURCE})

slicer3_set_plugins_output_path (${PROJECT_NAME}Module)

set_target_properties (${PROJECT_NAME}Module PROPERTIES
  COMPILE_FLAGS "-Dmain=ModuleEntryPoint")

target_link_libraries (${PROJECT_NAME}Module
  ITKIO
  ITKStatistics
  ITKBasicFilters
  vtkIO
  vtkImaging)


#-----------------------------------------------------------------------------
# Build executable

add_executable (${PROJECT_NAME}
  ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)

slicer3_set_plugins_output_path(${PROJECT_NAME})

target_link_libraries (${PROJECT_NAME} ${PROJECT_NAME}Module)


#-----------------------------------------------------------------------------
# Install

if (Slicer3_SOURCE_DIR)

  set(TARGETS ${PROJECT_NAME}Module ${PROJECT_NAME})

  slicer3_install_plugins(${TARGETS})

endif (Slicer3_SOURCE_DIR)


#-----------------------------------------------------------------------------
# Testing

if (BUILD_TESTING)

  #add_subdirectory(Testing)

endif (BUILD_TESTING)

