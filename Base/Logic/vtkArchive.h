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

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "vtkSlicerBaseLogicWin32Header.h"

// Warning, it extracts the archive into the current directory
// With Qt, you can change the current directory using QDir::setCurrent()
VTK_SLICER_BASE_LOGIC_EXPORT bool extract_tar(const char* outFileName, bool verbose, bool extract,
                                              std::vector<std::string> * extracted_files = 0);

#ifdef __cplusplus
}
#endif

#endif
