/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkArchive_h
#define __vtkArchive_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <string>
#include <vector>

/// \brief Simple class for manipulating archive files
///
class VTK_MRML_EXPORT vtkArchive : public vtkObject
{
public:
  static vtkArchive *New();
  vtkTypeMacro(vtkArchive, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static bool ListArchive(const char* archiveFileNameFileName, std::vector<std::string>& files);

  // Warning, it extracts the archive into the current directory
  // With Qt, you can change the current directory using QDir::setCurrent()
  static bool ExtractTar(const char* tarFileName, bool verbose, bool extract, std::vector<std::string> * extracted_files = nullptr);

  // creates a zip file with the full contents of the directory (recurses)
  // zip entries will include relative path of including tail of directoryToZip
  static bool Zip(const char* zipFileName, const char* directoryToZip);

  // unzips zip file into specified directory
  // (internally this supports many formats of archive, not just zip)
  static bool UnZip(const char* zipFileName, const char *destinationDirectory);

protected:
  vtkArchive();
  ~vtkArchive() override;
  vtkArchive(const vtkArchive&);
  void operator=(const vtkArchive&);
};

#endif
