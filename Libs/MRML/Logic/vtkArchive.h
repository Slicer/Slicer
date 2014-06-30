/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef __vtkArchive_h
#define __vtkArchive_h

// STD includes
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "vtkMRMLLogicWin32Header.h"

// TODO: this should really be a vtk class that has configuration options
// and progress events.

VTK_MRML_LOGIC_EXPORT bool list_archive(
  const char* archiveFileNameFileName,
  std::vector<std::string>& files);

// Warning, it extracts the archive into the current directory
// With Qt, you can change the current directory using QDir::setCurrent()
VTK_MRML_LOGIC_EXPORT bool extract_tar(const char* tarFileName, bool verbose, bool extract,
                                       std::vector<std::string> * extracted_files = 0);

// creates a zip file with the full contents of the directory (recurses)
// zip entries will include relative path of including tail of directoryToZip
VTK_MRML_LOGIC_EXPORT bool zip(const char* zipFileName, const char* directoryToZip);

// unzips zip file into specified directory
// (internally this supports many formats of archive, not just zip)
VTK_MRML_LOGIC_EXPORT bool unzip(const char* zipFileName, const char *destinationDirectory);
#ifdef __cplusplus
}
#endif

#endif
