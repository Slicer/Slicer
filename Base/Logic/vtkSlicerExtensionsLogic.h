#ifndef __vtkSlicerExtensionsLogic_h
#define __vtkSlicerExtensionsLogic_h

// Logic includes
#include "vtkSlicerBaseLogic.h"

// VTK includes
#include <vtkObject.h>

// STD includes
#include <string>
#include <vector>
#include <map>

/// Description:
/// An object to hold entry information.
class ManifestEntry
{
public:
  std::string Name;
  std::string Version;
  std::string URL;
  std::string Homepage;
  std::string Category;
  std::string ExtensionStatus;
  std::string Description;
  std::string Revision;
};

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerExtensionsLogic
  : public vtkObject
{
public:
  static vtkSlicerExtensionsLogic* New();
  vtkTypeRevisionMacro(vtkSlicerExtensionsLogic,vtkObject);

  const std::vector<ManifestEntry*>& GetModules()const;

  /// Helper method to read from disk and parse for modules
  bool UpdateModulesFromDisk(const std::string& modulePaths);

  /// Helper method to clear this->Modules
  void ClearModules();
  
  vtkSetStringMacro(InstallPath);
  vtkGetStringMacro(InstallPath);
  
  vtkSetStringMacro(RepositoryURL);
  vtkGetStringMacro(RepositoryURL);
  
  vtkSetStringMacro(TemporaryDirectory);
  vtkGetStringMacro(TemporaryDirectory);
      
  /// Helper method for dowlonad and install
  bool DownloadExtension(const std::string &ExtensionName,
                         const std::string &ExtensionBinaryURL);
  bool InstallExtension(const std::string &ExtensionName,
                        const std::string &ExtensionBinaryURL);
  bool UninstallExtension(const std::string &ExtensionName);

  /// Helper method to add greatest revision of named extensions to the vector.
  void AddEntry(std::vector<ManifestEntry*> &entries, ManifestEntry *entry);

  /// Helper method to download .s4ext file and parse
  bool DownloadAndParseS4ext(const std::string &s4ext, ManifestEntry *entry);

protected:
  vtkSlicerExtensionsLogic();
  virtual ~vtkSlicerExtensionsLogic();

  std::vector<ManifestEntry*> Modules;

private:
  vtkSlicerExtensionsLogic(const vtkSlicerExtensionsLogic&); /// Not implemented.
  void operator=(const vtkSlicerExtensionsLogic&); /// Not implemented.

  char* InstallPath;
  char* RepositoryURL;
  char* TemporaryDirectory;
};

#endif
