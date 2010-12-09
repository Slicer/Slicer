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
//BTX
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
//ETX

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerExtensionsLogic
  : public vtkObject
{
public:
  static vtkSlicerExtensionsLogic* New();
  vtkTypeRevisionMacro(vtkSlicerExtensionsLogic,vtkObject);

  /// 
  /// Get/Set status code for module
  //BTX
  enum
  {
    StatusSuccess = 0,
    StatusDownloading,
    StatusInstalling,
    StatusUninstalling,
    StatusCancelled,
    StatusError,
    StatusFoundOnDisk,
    StatusNotFoundOnDisk,
    StatusUnknown
  };
  //ETX

  //BTX
  const std::vector<ManifestEntry*>& GetModules()const;
  //ETX

  /// 
  /// Helper method to read from repository and parse for modules
  //BTX
  bool UpdateModulesFromRepository(const std::string& manifestFile);
  //ETX

  /// 
  /// Helper method to read from disk and parse for modules
  //BTX
  bool UpdateModulesFromDisk(const std::string& modulePaths);
  //ETX

  //Description:
  /// Helper method to clear this->Modules
  //BTX
  void ClearModules();
  //ETX
  
  vtkSetStringMacro(InstallPath);
  vtkGetStringMacro(InstallPath);
  
  vtkSetStringMacro(RepositoryURL);
  vtkGetStringMacro(RepositoryURL);
  
  vtkSetStringMacro(TemporaryDirectory);
  vtkGetStringMacro(TemporaryDirectory);
      
  /// 
  /// Helper method for dowlonad and install
  //BTX
  bool DownloadExtension(const std::string &ExtensionName,
                         const std::string &ExtensionBinaryURL);
  bool InstallExtension(const std::string &ExtensionName,
                        const std::string &ExtensionBinaryURL);
  bool UninstallExtension(const std::string &ExtensionName);
  //ETX

  //ETX

protected:
  vtkSlicerExtensionsLogic();
  virtual ~vtkSlicerExtensionsLogic();

  //BTX
  std::vector<ManifestEntry*> Modules;
  //ETX

  //BTX
  std::map<std::string, std::string> Messages;
  //ETX

private:
  vtkSlicerExtensionsLogic(const vtkSlicerExtensionsLogic&); /// Not implemented.
  void operator=(const vtkSlicerExtensionsLogic&); /// Not implemented.

  /// 
  /// Helper method to turn a module manifest into a selectable list.
  //BTX
  std::vector<ManifestEntry*> ParseManifest(const std::string&);
  //ETX

  /// 
  /// Helper method to add greatest revision of named extensions to the vector.
  //BTX
  void AddEntry(std::vector<ManifestEntry*> &entries, ManifestEntry *entry);
  //ETX

  /// 
  /// Helper method to download .s3ext file and parse
  //BTX
  bool DownloadParseS3ext(const std::string &s3ext, ManifestEntry *entry);
  //ETX

  /// 
  /// Helper method for adding extensions to the column list
  //BTX
  void InsertExtension(int Index,
                       ManifestEntry *Entry,
                       const std::string &InstallDir);
  //ETX

  /// 
  /// Helper method that asks the conf. step what action should be taken.
  //BTX
  int ActionToBeTaken();
  /// ETX

  /// 
  /// Keep track if anything was done or not.  Validation succeeds if
  /// at least one action was taken.
  //BTX
  int ActionTaken;
  //ETX

  char* InstallPath;
  char* RepositoryURL;
  char* TemporaryDirectory;
};

#endif
