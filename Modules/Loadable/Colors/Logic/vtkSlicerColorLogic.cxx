/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// Colors
#include "vtkSlicerColorLogic.h"

// Slicer
#include "vtkSlicerConfigure.h" // for Slicer_SHARE_DIR

// MRML
#include "vtkMRMLColorTableStorageNode.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <cerrno>
#endif

vtkCxxRevisionMacro(vtkSlicerColorLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerColorLogic);

//----------------------------------------------------------------------------
vtkSlicerColorLogic::vtkSlicerColorLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerColorLogic::~vtkSlicerColorLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerColorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkMRMLColorLogic::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultLabelMapColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultEditorColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericAnatomyColors.txt");
}

//----------------------------------------------------------------------------
const char *vtkSlicerColorLogic::GetDefaultChartColorNodeID()
{
  return vtkMRMLColorLogic::GetFileColorNodeID("GenericChartColors.txt");
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindDefaultColorFiles()
{
  // get the slicer home dir
  std::string slicerHome;
  if (vtksys::SystemTools::GetEnv("SLICER_HOME") == NULL)
    {
    if (vtksys::SystemTools::GetEnv("PWD") != NULL)
      {
      slicerHome =  std::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      slicerHome =  std::string("");
      }
    }
  else
    {
    slicerHome = std::string(vtksys::SystemTools::GetEnv("SLICER_HOME"));
    }
  // build up the vector
  std::vector<std::string> filesVector;
  filesVector.push_back(""); // for relative path
  filesVector.push_back(slicerHome);
  filesVector.push_back(std::string(Slicer_SHARE_DIR) + "/ColorFiles");
  std::string resourcesDirString = vtksys::SystemTools::JoinPath(filesVector);

  // now make up a vector to iterate through of dirs to look in
  std::vector<std::string> DirectoriesToCheck;

  DirectoriesToCheck.push_back(resourcesDirString);
  
  return this->FindColorFiles(DirectoriesToCheck);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindUserColorFiles()
{
  std::vector<std::string> DirectoriesToCheck;
  // add the list of dirs set from the application
  if (this->UserColorFilePaths != NULL)
    {
    vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
    // parse out the list, breaking at delimiter strings
#ifdef WIN32
    const char *delim = ";";
#else
    const char *delim = ":";
#endif
    char *ptr = strtok(this->UserColorFilePaths, delim);
    while (ptr != NULL)
      {
      std::string dir = std::string(ptr);
      vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
      DirectoriesToCheck.push_back(dir);
      ptr = strtok(NULL, delim);
      }
    } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }

  return this->FindColorFiles(DirectoriesToCheck);  
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerColorLogic::FindColorFiles(const std::vector<std::string>& directories)
{
  std::vector<std::string> filenames;
  
  // get the list of colour files in these dir
  for (unsigned int d = 0; d < directories.size(); d++)
    {
    std::string dirString = directories[d];
    vtkDebugMacro("FindColorFiles: checking for colour files in dir " << d << " = " << dirString.c_str());

    std::vector<std::string> filesVector;
    filesVector.push_back(dirString);
    filesVector.push_back(std::string("/"));

#ifdef WIN32
    WIN32_FIND_DATA findData;
    HANDLE fileHandle;
    int flag = 1;
    std::string search ("*.*");
    dirString += "/";
    search = dirString + search;

    fileHandle = FindFirstFile(search.c_str(), &findData);
    if (fileHandle != INVALID_HANDLE_VALUE)
      {
      while (flag)
        {
        // add this file to the vector holding the base dir name so check the
        // file type using the full path
        filesVector.push_back(std::string(findData.cFileName));
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dirString.c_str())) == NULL)
      {
      vtkErrorMacro("\nError(" << errno << ") opening user specified color path: " << dirString.c_str() << ", no color files will be loaded from that directory\n(check View -> Application Settings -> Module Settings to adjust your User defined color file paths)");
      }
    else
      {
      while ((dirp = readdir(dp)) != NULL)
        {
        // add this file to the vector holding the base dir name
        filesVector.push_back(std::string(dirp->d_name));
#endif

        std::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector);
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          // check that it's a supported file type
          // create a storage node so can check for supported file types
          vtkSmartPointer<vtkMRMLColorTableStorageNode>colorStorageNode =       vtkSmartPointer<vtkMRMLColorTableStorageNode>::New();
          if (colorStorageNode->SupportedFileType(fileToCheck.c_str()))
            {
            vtkDebugMacro("FindColorFiles: Adding " << fileToCheck.c_str() << " to list of potential colour files. Type = " << fileType);
              // add it to the list
              this->AddColorFile(fileToCheck.c_str(), &filenames);           
            }
          else
            {
            vtkWarningMacro("FindColorFiles: not a supported file type:\n"<< fileToCheck);
            }
          }
        else
          {
          vtkDebugMacro("FindColorFiles: Skipping potential colour file " << fileToCheck.c_str() << ", not a text file (file type = " << fileType << ")");
          }
        // take this file off so that can build the next file name
        filesVector.pop_back();

#ifdef WIN32
        flag = FindNextFile(fileHandle, &findData);
        } // end of while flag
      FindClose(fileHandle);
      } // end of having a valid fileHandle
#else
        } // end of while loop over reading the directory entries
      closedir(dp);
      } // end of able to open dir
#endif
    } // end of looping over dirs
  return filenames;
}
