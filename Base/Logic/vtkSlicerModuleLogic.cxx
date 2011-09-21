/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkSlicerModuleLogic.h"

#include <vtksys/SystemTools.hxx>


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
