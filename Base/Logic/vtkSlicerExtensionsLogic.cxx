// Logic includes
#include "vtkSlicerExtensionsLogic.h"

// vtkSlicer includes
#include "vtkSlicerConfigure.h"
#include "vtkArchive.h"

// RemoteIO includes
#include <vtkHTTPHandler.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// ITKsys includes
#include <itksys/SystemTools.hxx>
#include <vtksys/ios/sstream>

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
bool UnzipPackage(const std::string& zipfile, 
                  const std::string& vtkNotUsed(target),
                  const std::string& vtkNotUsed(tmpdir))
{
  return extract_tar(zipfile.c_str(), true, true);
}

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerExtensionsLogic );
vtkCxxRevisionMacro(vtkSlicerExtensionsLogic, "$Revision: 12830 $");

//----------------------------------------------------------------------------
vtkSlicerExtensionsLogic::vtkSlicerExtensionsLogic()
{
  this->Messages["READY"] = "Select extensions, then click uninstall to remove them from\nyour version of 3D Slicer, or click download to retrieve them.";
  this->Messages["DOWNLOAD"] = "Download in progress... Clicking the cancel button will stop\nthe process after the current extension operation is finished.";
  this->Messages["FINISHED"] = "Continue selecting extensions for download or removal,\nor click finish to complete the operation.";

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

//--------------------vtkNotUsed(--------------------------------------------------------
bool vtkSlicerExtensionsLogic
::UpdateModulesFromRepository(const std::string& manifestFile)
{
  std::ifstream ifs(manifestFile.c_str());

  if (ifs.fail())
    {
    return false;
    }

  char *HTML = 0;
  ifs.seekg(0, std::ios::end);
  size_t len = ifs.tellg();
  ifs.seekg(0, std::ios::beg);
  HTML = new char[len+1];
  ifs.read(HTML, len);
  HTML[len] = '\n';
  
  ifs.close();

  std::vector<ManifestEntry*> modules = this->ParseManifest(HTML);
  this->Modules.insert(this->Modules.begin(), modules.begin(), modules.end());

  delete[] HTML;

  return modules.size() > 0;
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
std::vector<ManifestEntry*> vtkSlicerExtensionsLogic::ParseManifest(const std::string& txt)
{
  std::vector<ManifestEntry*> result;

  if (txt.empty()) {
    return result;
  }

  std::string baseURL = this->GetRepositoryURL();

  std::string zip_key(".zip\">");
  std::string ext_key(".s3ext\">");
  std::string atag_key("</a>");
  std::string svn_key("-svn");
  std::string cvs_key("-cvs");

  std::string::size_type zip = txt.find(zip_key, 0);
  std::string::size_type atag = txt.find(atag_key, zip);
  std::string::size_type dash = txt.find(svn_key, zip);

  bool cvs = false;
  if (std::string::npos == dash || dash > atag)
    {
    cvs = true;
    dash = txt.find(cvs_key, zip);
    }

  std::string::size_type dash2;
  if (cvs)
    {
    dash2 = (dash + 3 + 10);
    }
  else
    {
    dash2 = txt.find("-", dash + 1);
    }

  std::string::size_type ext = txt.find(ext_key, dash2);
  std::string::size_type atag2 = txt.find(atag_key, ext);

  ManifestEntry* entry;

  // :NOTE: 20081003 tgl: Put in a sanity check of 10,000 to
  // prevent an infinite loop.  Get Out The Vote 2008!

  std::string s3ext;
  int count = 0;
  while (zip != std::string::npos && count < 10000)
    {
    entry = new ManifestEntry;

    if (std::string::npos != atag2)
      {
      entry->URL = baseURL;
      entry->URL += "/";
      entry->URL += txt.substr(zip + zip_key.size(), atag - (zip + zip_key.size()));
      
      if (cvs)
        {
        entry->Name = txt.substr(zip + zip_key.size(), dash - (zip + zip_key.size()));
        }
      else
        {
        entry->Name = txt.substr(zip + zip_key.size(), dash - (zip + zip_key.size()));
        }

      if (cvs)
        {
        // :NOTE: 20090519 tgl: CVS controlled extensions use an ISO date.
        entry->Revision = txt.substr(dash + cvs_key.size(), 10);
        }
      else
        {
        entry->Revision = txt.substr(dash + svn_key.size(), dash2 - (dash + svn_key.size()));
        }

      s3ext = baseURL;
      s3ext += "/";
      s3ext += txt.substr(ext + ext_key.size(), atag2 - (ext + ext_key.size()));

      this->DownloadParseS3ext(s3ext, entry);

      zip = txt.find(zip_key, zip + 1);

      dash = txt.find(svn_key, zip);
      atag = txt.find(atag_key, zip);

      cvs = false;
      if (std::string::npos == dash || dash > atag)
        {
        cvs = true;
        dash = txt.find(cvs_key, zip);
        }

      dash2 = txt.find("-", dash + 1);
      ext = txt.find(ext_key, dash );
      atag2 = txt.find(atag_key, ext);

      this->AddEntry(result, entry);
      }
      
    count++;
    }

  return result;
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
  
}// AddEntry

//----------------------------------------------------------------------------
bool vtkSlicerExtensionsLogic::DownloadParseS3ext(const std::string& s3ext,
                                              ManifestEntry* entry)
{
  vtkSmartPointer<vtkHTTPHandler> handler = vtkSmartPointer<vtkHTTPHandler>::New();
  handler->SetForbidReuse(1);

  if (handler->CanHandleURI(s3ext.c_str()) == 0)
    {
    return false;
    }
  std::string::size_type pos = s3ext.rfind("/");
  std::string s3extname = s3ext.substr(pos + 1);
    
  std::string tmpfile(std::string(this->GetTemporaryDirectory()) + std::string("/") + s3extname);
    
  handler->StageFileRead(s3ext.c_str(), tmpfile.c_str());

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
bool vtkSlicerExtensionsLogic::InstallExtension(const std::string& ExtensionName,
                                                const std::string& ExtensionBinaryURL)
{
  this->UninstallExtension(ExtensionName);

  std::string::size_type pos = ExtensionBinaryURL.rfind("/");
  std::string zipname = ExtensionBinaryURL.substr(pos + 1);     

  std::string tmpfile(std::string(this->GetTemporaryDirectory()) + std::string("/") + zipname);
  
  std::string installdir = this->GetInstallPath();

  std::string libdir(installdir + std::string("/") + ExtensionName);

  std::string searchdir(std::string(this->GetInstallPath()) + std::string("/") + ExtensionName);

  std::string tmpdir(std::string(this->GetTemporaryDirectory()) + std::string("/extension"));

  if (!UnzipPackage(tmpfile, libdir, tmpdir))
    {
    std::cerr << "Can't extract zip file" << std::endl;
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
