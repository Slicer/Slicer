/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerModuleLogic.h"

#include <vtksys/SystemTools.hxx>

#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#endif

#include "vtkSlicerConfigure.h" /* for Slicer_* */

vtkCxxRevisionMacro(vtkSlicerModuleLogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::vtkSlicerModuleLogic()
{
  this->ModuleLocation = NULL;
  this->ModuleShareDirectory = NULL;
  this->ModuleLibDirectory = NULL;
  this->ModuleName = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::~vtkSlicerModuleLogic()
{
  this->SetApplicationLogic(NULL);
  this->SetModuleLocation(NULL);
  this->SetModuleShareDirectory(NULL);
  this->SetModuleLibDirectory(NULL);
  this->SetModuleName(NULL);
//----------------------------------------------------------------------------
bool vtkSlicerModuleLogic::IsExtension(const std::string& filePath, const std::string& applicationHomeDir)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return false;
    }
  if (applicationHomeDir.empty())
    {
    vtkGenericWarningMacro( << "applicationHomeDir is an empty string !");
    return false;
    }
  std::string extensionPath = vtksys::SystemTools::GetFilenamePath(filePath);
  return !vtksys::SystemTools::StringStartsWith(extensionPath.c_str(), applicationHomeDir.c_str());
}

//----------------------------------------------------------------------------
bool vtkSlicerModuleLogic::IsPluginInstalled(const std::string& filePath,
                                             const std::string& applicationHomeDir)
{
  if (filePath.empty())
    {
    vtkGenericWarningMacro( << "filePath is an empty string !");
    return false;
    }
  if (applicationHomeDir.empty())
    {
    vtkGenericWarningMacro( << "applicationHomeDir is an empty string !");
    return false;
    }

  std::string path = vtksys::SystemTools::GetFilenamePath(filePath);
  std::string canonicalPath = vtksys::SystemTools::GetRealPath(path.c_str());

  if (vtksys::SystemTools::StringStartsWith(canonicalPath.c_str(), applicationHomeDir.c_str()))
    {
    return !vtksys::SystemTools::FileExists(
          std::string(applicationHomeDir).append("/CMakeCache.txt").c_str(), true);
    }

  std::string root;
  std::string canonicalPathWithoutRoot =
      vtksys::SystemTools::SplitPathRootComponent(canonicalPath.c_str(), &root);
  do
    {
    if (vtksys::SystemTools::FileExists(
          (root + canonicalPathWithoutRoot + "/CMakeCache.txt").c_str(), true))
      {
      return false;
      }
    canonicalPathWithoutRoot = vtksys::SystemTools::GetParentDirectory(canonicalPathWithoutRoot.c_str());
    }
  while(!canonicalPathWithoutRoot.empty());

  return true;
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic* vtkSlicerModuleLogic::GetApplicationLogic()
{
  return vtkSlicerApplicationLogic::SafeDownCast(this->GetMRMLApplicationLogic());
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::SetApplicationLogic(vtkSlicerApplicationLogic* logic)
{
  this->SetMRMLApplicationLogic(logic);
}

//----------------------------------------------------------------------------
const char* vtkSlicerModuleLogic::GetModuleShareDirectory()
{
  if (!this->ModuleShareDirectory)
    {
    if (this->ModuleLocation)
      {
      vtksys_stl::string library_location(this->ModuleLocation);
      vtksys::SystemTools::ConvertToUnixSlashes(library_location);
      vtksys_stl::string library_directory = 
        vtksys::SystemTools::GetFilenamePath(library_location.c_str());
      vtksys_stl::string module_name = 
        vtksys::SystemTools::GetFilenameWithoutExtension(
          library_location.c_str());
#ifndef _WIN32
      // If the module was a library, try to guess the module name from
      // the library name, without the 'lib' prefix.
      if (module_name.size() > 3 && 
          !strncmp(module_name.c_str(), "lib", 3))
        {
        module_name.erase(0, 3);
        }
#endif

#ifdef _WIN32
      vtksys_stl::string intdir = 
        vtksys::SystemTools::GetFilenameName(library_directory.c_str());
      if (!vtksys::SystemTools::Strucmp(intdir.c_str(), "Debug") ||
          !vtksys::SystemTools::Strucmp(intdir.c_str(), "Release") ||
          !vtksys::SystemTools::Strucmp(intdir.c_str(), "RelWithDebInfo") ||
          !vtksys::SystemTools::Strucmp(intdir.c_str(), "MinSizeRel"))
        {
        library_directory = 
          vtksys::SystemTools::GetFilenamePath(library_directory.c_str());
        }
#endif
#ifdef Slicer_QTLOADABLEMODULES_LIB_DIR
      vtksys::SystemTools::ReplaceString(
        library_directory,
        Slicer_QTLOADABLEMODULES_LIB_DIR,
        Slicer_QTLOADABLEMODULES_SHARE_DIR
        );
#endif
      vtksys_stl::string share_directory = 
        library_directory + "/" + module_name;
      this->SetModuleShareDirectory(share_directory.c_str());
      }
    else
      {
      const char *env = vtksys::SystemTools::GetEnv("SLICER_HOME");
      if (!env)
        {
        env = vtksys::SystemTools::GetEnv("PWD");
        }
      if (env)
        {
        vtksys_stl::string directory(env);
        vtksys::SystemTools::ConvertToUnixSlashes(directory);
#ifdef Slicer_QTLOADABLEMODULES_SHARE_DIR
        directory = directory + '/' + Slicer_QTLOADABLEMODULES_SHARE_DIR;
#else
        directory = directory + "/share";
#endif
        if (this->GetModuleName())
          {
          directory = directory + '/' + this->GetModuleName();
          }
        this->SetModuleShareDirectory(directory.c_str());
        }
      }
    }

  return this->ModuleShareDirectory;
}

//----------------------------------------------------------------------------
int vtkSlicerModuleLogic::IsModuleShareDirectoryDefined()
{
  return this->ModuleShareDirectory ? 1 : 0;
}

//----------------------------------------------------------------------------
const char* vtkSlicerModuleLogic::GetModuleLibDirectory()
{
  if (!this->ModuleLibDirectory)
    {
    const char *share_dir = this->GetModuleShareDirectory();
    if (share_dir)
      {
      vtksys_stl::string directory(share_dir);
#ifdef Slicer_QTLOADABLEMODULES_SHARE_DIR
      vtksys::SystemTools::ReplaceString(
        directory,
        Slicer_QTLOADABLEMODULES_SHARE_DIR,
        Slicer_QTLOADABLEMODULES_LIB_DIR
        );
#endif
      this->SetModuleLibDirectory(directory.c_str());
      }
    }

  return this->ModuleLibDirectory;
}

//----------------------------------------------------------------------------
int vtkSlicerModuleLogic::IsModuleLibDirectoryDefined()
{
  return this->ModuleLibDirectory ? 1 : 0;
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "SlicerLogic:             " << this->GetClassName() << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::LoadDefaultParameterSets(vtkMRMLScene *scene)
{
  // get the slicer home dir
  vtksys_stl::string slicerHome;
  if (vtksys::SystemTools::GetEnv("SLICER_HOME") == NULL)
    {
    if (vtksys::SystemTools::GetEnv("PWD") != NULL)
      {
      slicerHome =  vtksys_stl::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      slicerHome =  vtksys_stl::string("");
      }
    }
  else
    {
    slicerHome = vtksys_stl::string(vtksys::SystemTools::GetEnv("SLICER_HOME"));
    }
  // build up the vector
  vtksys_stl::vector<vtksys_stl::string> filesVector;
  vtksys_stl::vector<vtksys_stl::string> filesToLoad;
  filesVector.push_back(""); // for relative path
  filesVector.push_back(slicerHome);
  filesVector.push_back(vtksys_stl::string(Slicer_SHARE_DIR) + "/ParameterSets");
  vtksys_stl::string resourcesDirString = vtksys::SystemTools::JoinPath(filesVector);

  // now make up a vector to iterate through of dirs to look in
  std::vector<std::string> DirectoriesToCheck;

  DirectoriesToCheck.push_back(resourcesDirString);

// Didn't port this next block of code yet.  Would need to add a
//   UserParameterSetsPath to the object and some window
//
//   // add the list of dirs set from the application
//   if (this->UserColorFilePaths != NULL)
//     {
//     vtkDebugMacro("\nFindColorFiles: got user color file paths = " << this->UserColorFilePaths);
//     // parse out the list, breaking at delimiter strings
// #ifdef WIN32
//     const char *delim = ";";
// #else
//     const char *delim = ":";
// #endif
//     char *ptr = strtok(this->UserColorFilePaths, delim);
//     while (ptr != NULL)
//       {
//       std::string dir = std::string(ptr);
//       vtkDebugMacro("\nFindColorFiles: Adding user dir " << dir.c_str() << " to the directories to check");
//       DirectoriesToCheck.push_back(dir);
//       ptr = strtok(NULL, delim);     
//       }
//     } else { vtkDebugMacro("\nFindColorFiles: oops, the user color file paths aren't set!"); }
  
  
  // get the list of parameter sets in these dir
  for (unsigned int d = 0; d < DirectoriesToCheck.size(); d++)
    {
    vtksys_stl::string dirString = DirectoriesToCheck[d];
    //vtkDebugMacro("\nLoadDefaultParameterSets: checking for parameter sets in dir " << d << " = " << dirString.c_str());

    filesVector.clear();
    filesVector.push_back(dirString);
    filesVector.push_back(vtksys_stl::string("/"));

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
        filesVector.push_back(vtksys_stl::string(findData.cFileName));
#else
    DIR *dp;
    struct dirent *dirp;
    if ((dp  = opendir(dirString.c_str())) == NULL)
      {
      vtkGenericWarningMacro("Error(" << errno << ") opening " << dirString.c_str());
      }
    else
      {
      while ((dirp = readdir(dp)) != NULL)
        {
        // add this file to the vector holding the base dir name
        filesVector.push_back(vtksys_stl::string(dirp->d_name));
#endif
        
        vtksys_stl::string fileToCheck = vtksys::SystemTools::JoinPath(filesVector); 
        int fileType = vtksys::SystemTools::DetectFileType(fileToCheck.c_str());
        if (fileType == vtksys::SystemTools::FileTypeText)
          {
          //vtkDebugMacro("\nAdding " << fileToCheck.c_str() << " to list of potential parameter sets. Type = " << fileType);
          filesToLoad.push_back(fileToCheck);
          }
        else
          {
          //vtkDebugMacro("\nSkipping potential parameter set " << fileToCheck.c_str() << ", file type = " << fileType);
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

  // Save the URL and root directory of the scene so it can
  // be restored after loading presets
  std::string url = scene->GetURL();
  std::string rootdir = scene->GetRootDirectory();

  // Finally, load each of the parameter sets
  vtksys_stl::vector<vtksys_stl::string>::iterator fit;
  for (fit = filesToLoad.begin(); fit != filesToLoad.end(); ++fit)
    {
    scene->SetURL( (*fit).c_str() );
    scene->Import();
    }

  // restore URL and root dir
  scene->SetURL(url.c_str());
  scene->SetRootDirectory(rootdir.c_str());
}
