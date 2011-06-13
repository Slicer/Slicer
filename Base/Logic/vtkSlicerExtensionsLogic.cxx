// Logic includes
#include "vtkSlicerExtensionsLogic.h"

// vtkSlicer includes
#include "vtkSlicerConfigure.h"
#include "vtkArchive.h"

// RemoteIO includes
#include <vtkHTTPHandler.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// ITKsys includes
#include <itksys/SystemTools.hxx>
#include <vtksys/ios/sstream>

namespace
{

//----------------------------------------------------------------------------
std::string ltrim(const std::string& str)
{
  std::string s(str);
  if (s.size())
    {
    std::string::size_type pos = s.find_first_not_of(" \n\t");
    if (pos != std::string::npos)
      {
      s = s.substr(pos);
      }
    else
      {
      s = "";
      }
    }
  return s;
}

//----------------------------------------------------------------------------
std::string rtrim(const std::string& str)
{
  std::string s(str);
  if (s.size())
    {
    std::string::size_type pos = s.find_last_not_of(" \n\t");
    if (pos != std::string::npos)
      {
      s = s.substr(0, pos + 1);
      }
    else
      {
      s = "";
      }
    }
  return s;
}

//----------------------------------------------------------------------------
std::string trim(const std::string& str)
{
  return rtrim(ltrim(str));
}

//----------------------------------------------------------------------------
bool ExtractPackage(const std::string& file,
                  const std::string& vtkNotUsed(target),
                  const std::string& vtkNotUsed(tmpdir))
{
  return extract_tar(file.c_str(), true, true);
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
// vtkSlicerExtensionsLogic methods

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerExtensionsLogic);
vtkCxxRevisionMacro(vtkSlicerExtensionsLogic, "$Revision: 12830 $");

//----------------------------------------------------------------------------
vtkSlicerExtensionsLogic::vtkSlicerExtensionsLogic()
{
  this->InstallPath = 0;
  this->RepositoryURL = 0;
  this->TemporaryDirectory = 0;
}

//----------------------------------------------------------------------------
vtkSlicerExtensionsLogic::~vtkSlicerExtensionsLogic()
{
  this->ClearModules();
}

//----------------------------------------------------------------------------
const std::vector<ManifestEntry*>& vtkSlicerExtensionsLogic::GetModules()const
{
  return this->Modules;
}

//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::UpdateModulesFromDisk(const std::string& paths)
{
  std::string::size_type oldSize = this->Modules.size();

  std::cout << "UPDATE paths: " << paths << std::endl;

  const char delim = ';';
  const char dir =  '/';

  ManifestEntry* entry;

  std::string::size_type npos = paths.find(delim);

  std::cout << "npos: " << npos << std::endl;

  std::string::size_type dpos = paths.rfind(dir, npos);
  ++dpos;
  std::cout << "dpos: " << dpos << std::endl;

  while (std::string::npos != npos)
    {
    entry = new ManifestEntry;

    entry->Name = paths.substr(dpos, npos - dpos);
    
    std::cout << "name: " << entry->Name << std::endl;

    this->AddEntry(this->Modules, entry);

    npos = paths.find(delim, npos + 1);
    dpos = paths.rfind(dir, npos);
    if (dpos == std::string::npos)
      {
      dpos = paths.rfind(delim, npos - 1);
      }
    ++dpos;

    std::cout << "npos: " << npos << std::endl;
    std::cout << "dpos: " << dpos << std::endl;
    }
  
  if (paths.length() > 0)
    {
    npos = paths.length();
    dpos = paths.rfind(dir, npos);
    if (dpos == std::string::npos)
      {
      dpos = paths.rfind(delim, npos - 1);
      }
    ++dpos;

    std::cout << "npos: " << npos << std::endl;
    std::cout << "dpos: " << dpos << std::endl;

    entry = new ManifestEntry;

    entry->Name = paths.substr(dpos, npos - dpos);
  
    std::cout << "name: " << entry->Name << std::endl;
  
    this->AddEntry(this->Modules, entry);
    }

  return this->Modules.size() > oldSize;
}

//----------------------------------------------------------------------------
void vtkSlicerExtensionsLogic::AddEntry(std::vector<ManifestEntry*> &entries,
                                        ManifestEntry *entry)
{
  std::vector<ManifestEntry*>::iterator iter;
  for (iter = entries.begin(); iter != entries.end(); iter++)
    {
    if ((*iter)->Name == entry->Name)
      {
      break;
      }
    }
  if (iter == entries.end())
    {
    entries.push_back(entry);
    }
  else
    {
    if (entry->Revision > (*iter)->Revision)
      {
      delete (*iter);
      (*iter) = entry;
      }
    else
      {
      delete entry;
      entry = 0;
      }
    }
}

//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::DownloadAndParseS4ext(const std::string& s4ext,
                                                     ManifestEntry* entry)
{
  vtkNew<vtkHTTPHandler> handler;
  handler->SetForbidReuse(1);

  if (handler->CanHandleURI(s4ext.c_str()) == 0)
    {
    return false;
    }
  std::string::size_type pos = s4ext.rfind("/");
  std::string s3extname = s4ext.substr(pos + 1);
    
  std::string tmpfile(std::string(this->GetTemporaryDirectory()) + std::string("/") + s3extname);
    
  handler->StageFileRead(s4ext.c_str(), tmpfile.c_str());

  std::ifstream ifs(tmpfile.c_str());
  std::string line;
  
  while (std::getline(ifs, line))
    {
    if (line.find("homepage") == 0)
      {
      entry->Homepage = trim(line.substr(9));
      }
    else if (line.find("category") == 0)
      {
      entry->Category = trim(line.substr(9));
      }
    else if (line.find("status") == 0)
      {
      entry->ExtensionStatus = trim(line.substr(7));
      }
    else if (line.find("description") == 0)
      {
      entry->Description = trim(line.substr(12));
      }
    }

  ifs.close();
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::DownloadExtension(const std::string& vtkNotUsed(ExtensionName),
                                                 const std::string& ExtensionBinaryURL)
{
  vtkSmartPointer<vtkHTTPHandler> handler = vtkSmartPointer<vtkHTTPHandler>::New();
  handler->SetForbidReuse(1);

  if (handler->CanHandleURI(ExtensionBinaryURL.c_str()) == 0)
    {
    return false;
    }

  std::string::size_type pos = ExtensionBinaryURL.rfind("/");
  std::string zipname = ExtensionBinaryURL.substr(pos + 1);
     
  std::string tmpfile(std::string(this->GetTemporaryDirectory()) + std::string("/") + zipname);
    
  handler->StageFileRead(ExtensionBinaryURL.c_str(), tmpfile.c_str());
  
  return true;
}


//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::InstallExtension(const std::string& extensionName,
                                                const std::string& extensionBinaryURL)
{
  this->UninstallExtension(extensionName);

  std::string::size_type pos = extensionBinaryURL.rfind("/");
  std::string archivename = extensionBinaryURL.substr(pos + 1);

  std::string tmpfile(std::string(this->GetTemporaryDirectory()) + std::string("/") + archivename);
  
  std::string installdir = this->GetInstallPath();

  std::string libdir(installdir + std::string("/") + extensionName);

//  std::string searchdir(std::string(this->GetInstallPath()) + std::string("/") + extensionName);

  std::string tmpdir(std::string(this->GetTemporaryDirectory()) + std::string("/extension"));

  if (!ExtractPackage(tmpfile, libdir, tmpdir))
    {
    std::cerr << "Can't extract extension package" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::UninstallExtension(const std::string& ExtensionName)
{
  bool result = false;

  // :BUG: 20090108 tgl: Not guaranteed that the install of the
  // module will be under this directory if the user has made edits
  // to the location over time.

  std::string installdir = this->GetInstallPath();

  std::string libdir(installdir + std::string("/") + ExtensionName);
  
  if (itksys::SystemTools::FileExists(libdir.c_str()))
    {
    itksys::SystemTools::RemoveADirectory(libdir.c_str());
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkSlicerExtensionsLogic::ClearModules()
{
  std::vector<ManifestEntry*>::iterator iter = this->Modules.begin();
  while (iter != this->Modules.end())
    {
    delete (*iter);
    iter++;
    }
  this->Modules.clear();
}
