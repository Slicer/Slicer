
#include "vtkCacheManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"

#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

// STD includes
#include <algorithm>
#include <cstring>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

vtkStandardNewMacro(vtkCacheManager);

#define MB 1000000.0

namespace
{
bool IsNullOrEmpty(const char* value)
{
  return value == nullptr || value[0] == '\0';
}

bool IsDirectoryNavigationEntry(const char* fileName)
{
  return std::strcmp(fileName, ".") == 0 || std::strcmp(fileName, "..") == 0;
}
} // namespace

//----------------------------------------------------------------------------
std::string vtkCacheManager::CanonicalizePath(const char* path) const
{
  if (IsNullOrEmpty(path))
  {
    return std::string();
  }

  std::string canonicalPath = vtksys::SystemTools::GetRealPath(path);
  if (canonicalPath.empty())
  {
    canonicalPath = vtksys::SystemTools::CollapseFullPath(path);
  }
  return canonicalPath;
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::JoinPath(const std::string& directoryPath, const std::string& fileName) const
{
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath(directoryPath, pathComponents);
  pathComponents.push_back(fileName);
  return vtksys::SystemTools::JoinPath(pathComponents);
}

//----------------------------------------------------------------------------
bool vtkCacheManager::PathIsWithinDirectory(const std::string& directoryPath, const std::string& path) const
{
  if (directoryPath.empty() || path.empty())
  {
    return false;
  }

  std::string relativePath = vtksys::SystemTools::RelativePath(directoryPath.c_str(), path.c_str());
  if (relativePath.empty())
  {
    return directoryPath == path;
  }

  if (relativePath == ".")
  {
    return true;
  }

  if (vtksys::SystemTools::FileIsFullPath(relativePath.c_str()))
  {
    return false;
  }

  return !(relativePath == ".." || relativePath.rfind("../", 0) == 0 || relativePath.rfind("..\\", 0) == 0);
}

//----------------------------------------------------------------------------
bool vtkCacheManager::IsDirectoryEmpty(const std::string& directoryPath) const
{
  vtksys::Directory directory;
  if (directoryPath.empty() || !directory.Load(directoryPath))
  {
    return false;
  }

  for (size_t fileNum = 0; fileNum < directory.GetNumberOfFiles(); ++fileNum)
  {
    const char* fileName = directory.GetFile(static_cast<unsigned long>(fileNum));
    if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
    {
      continue;
    }
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::GetSentinelFilePath(const std::string& cacheDirectory) const
{
  if (cacheDirectory.empty())
  {
    return std::string();
  }
  return this->JoinPath(cacheDirectory, this->GetSentinelFileName());
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::GetSentinelFilePath(const char* cacheDirectory) const
{
  if (IsNullOrEmpty(cacheDirectory))
  {
    return std::string();
  }
  return this->JoinPath(cacheDirectory, this->GetSentinelFileName());
}

//----------------------------------------------------------------------------
bool vtkCacheManager::HasSentinelFileInDirectory(const std::string& cacheDirectory) const
{
  std::string sentinelFilePath = this->GetSentinelFilePath(cacheDirectory);
  return !sentinelFilePath.empty() && vtksys::SystemTools::FileExists(sentinelFilePath.c_str());
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CreateSentinelFileInDirectory(const std::string& cacheDirectory) const
{
  if (cacheDirectory.empty() || !vtksys::SystemTools::FileIsDirectory(cacheDirectory))
  {
    return false;
  }

  std::string sentinelFilePath = this->GetSentinelFilePath(cacheDirectory);
  if (sentinelFilePath.empty())
  {
    return false;
  }
  if (vtksys::SystemTools::FileExists(sentinelFilePath.c_str()))
  {
    return true;
  }

  return vtksys::SystemTools::Touch(sentinelFilePath, true).IsSuccess();
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CanDeletePathInCache(const char* cacheDirectory, const char* pathToDelete, std::string& reason) const
{
  reason.clear();
  if (IsNullOrEmpty(cacheDirectory))
  {
    reason = "cache directory is empty";
    return false;
  }
  if (IsNullOrEmpty(pathToDelete))
  {
    reason = "path to delete is empty";
    return false;
  }

  if (!this->HasSentinelFileInDirectory(cacheDirectory))
  {
    reason = std::string("cache sentinel file is missing (") + this->GetSentinelFileName() + ")";
    return false;
  }

  std::string canonicalCachePath = this->CanonicalizePath(cacheDirectory);
  std::string canonicalDeletePath = this->CanonicalizePath(pathToDelete);

  if (canonicalCachePath.empty() || canonicalDeletePath.empty())
  {
    reason = "unable to resolve canonical path";
    return false;
  }

  if (!this->PathIsWithinDirectory(canonicalCachePath, canonicalDeletePath))
  {
    reason = "path resolves outside the cache directory";
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CollectCachedFilesRecursively(const std::string& directoryPath,
                                                    const std::string& rootDirectoryPath,
                                                    std::vector<vtkCacheManagerCachedFile>& cachedFiles) const
{
  if (!vtksys::SystemTools::FileIsDirectory(directoryPath.c_str()))
  {
    return false;
  }

  std::string canonicalRootDirectory;
  if (!rootDirectoryPath.empty())
  {
    canonicalRootDirectory = this->CanonicalizePath(rootDirectoryPath.c_str());
  }

  std::set<std::string> visitedCanonicalDirectories;
  std::vector<std::string> pendingDirectories;
  pendingDirectories.push_back(directoryPath);

  while (!pendingDirectories.empty())
  {
    const std::string currentDirectoryPath = pendingDirectories.back();
    pendingDirectories.pop_back();

    if (!vtksys::SystemTools::FileIsDirectory(currentDirectoryPath.c_str()))
    {
      continue;
    }

    std::string canonicalCurrentDirectory = this->CanonicalizePath(currentDirectoryPath.c_str());
    if (!canonicalCurrentDirectory.empty())
    {
      if (!visitedCanonicalDirectories.insert(canonicalCurrentDirectory).second)
      {
        // Skipping already visited cache directory (possible symlink/junction cycle)
        continue;
      }
    }

    vtksys::Directory directory;
    if (!directory.Load(currentDirectoryPath.c_str()))
    {
      continue;
    }

    for (size_t fileNum = 0; fileNum < directory.GetNumberOfFiles(); ++fileNum)
    {
      const char* fileName = directory.GetFile(static_cast<unsigned long>(fileNum));
      if (IsDirectoryNavigationEntry(fileName))
      {
        continue;
      }

      if (this->GetSentinelFileName() == fileName)
      {
        continue;
      }

      std::string fullPath = this->JoinPath(currentDirectoryPath, fileName);

      std::string canonicalPath = this->CanonicalizePath(fullPath.c_str());

      if (!canonicalPath.empty() && !canonicalRootDirectory.empty() && !this->PathIsWithinDirectory(canonicalRootDirectory, canonicalPath))
      {
        vtkWarningMacro("Skipping cache entry outside of traversal root: " << fullPath);
        continue;
      }

      if (vtksys::SystemTools::FileIsDirectory(fullPath.c_str()))
      {
        pendingDirectories.push_back(fullPath);
        continue;
      }

      vtkCacheManagerCachedFile cachedFile;
      cachedFile.Path = fullPath;
      cachedFile.ModifiedTime = static_cast<long long>(vtksys::SystemTools::ModifiedTime(fullPath));
      cachedFiles.push_back(cachedFile);
    }
  }

  return true;
}

//----------------------------------------------------------------------------
vtkCacheManager::vtkCacheManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->RemoteCacheLimit = 1000; // MB (a 3D image is around a few hundred MB, so this allows at least a few files to be cached)
  this->CurrentCacheSize = 0;    // MB
  this->EnableForceRedownload = 0;
  this->InsufficientFreeBufferNotificationFlag = 0;
  this->UpdatingCacheInformation = false;
}

//----------------------------------------------------------------------------
vtkCacheManager::~vtkCacheManager()
{
  if (this->CallbackCommand)
  {
    this->CallbackCommand->Delete();
  }
}

//----------------------------------------------------------------------------
const char* vtkCacheManager::GetFileFromURIMap(const char* uri)
{
  std::string uriString(uri);

  //--- URI is first, local name is second
  std::map<std::string, std::string>::iterator iter = this->UriMap.find(uriString);
  if (iter != this->UriMap.end())
  {
    return iter->second.c_str();
  }

  return nullptr;
}

//----------------------------------------------------------------------------
void vtkCacheManager::MapFileToURI(const char* uri, const char* fname)
{
  if (uri == nullptr || fname == nullptr)
  {
    vtkErrorMacro("MapFileToURI: got two null strings.");
    return;
  }

  std::string remote(uri);
  std::string local(fname);

  std::map<std::string, std::string>::iterator iter = this->UriMap.find(remote);
  if (iter != this->UriMap.end())
  {
    iter->second = local;
  }
  else
  {
    this->UriMap.insert(std::make_pair(remote, local));
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkCacheManager::SetRemoteCacheDirectory(const char* dir)
{
  if (dir == nullptr)
  {
    vtkWarningMacro("Setting RemoteCacheDirectory from null pointer is not allowed.");
    return;
  }

  std::string dirstring = dir;
  if (!dirstring.empty())
  {
    // make sure to remove backslash on the end of the dirstring.
    const char lastChar = dirstring.at(dirstring.length() - 1);
    if (lastChar == '/' || lastChar == '\\')
    {
      dirstring = dirstring.substr(0, dirstring.length() - 1);
    }
    std::string canonicalDirectory = this->CanonicalizePath(dirstring.c_str());
    if (!canonicalDirectory.empty())
    {
      dirstring = canonicalDirectory;
    }
  }

  if (this->RemoteCacheDirectory == dirstring)
  {
    // no change
    return;
  }

  this->RemoteCacheDirectory = dirstring;

  if (!dirstring.empty())
  {
    if (!vtksys::SystemTools::FileExists(this->RemoteCacheDirectory.c_str()))
    {
      if (!vtksys::SystemTools::MakeDirectory(this->RemoteCacheDirectory.c_str()))
      {
        vtkErrorMacro("Failed to create cache directory " << this->RemoteCacheDirectory << ".");
      }
      else
      {
        if (!this->CreateSentinelFileInDirectory(this->RemoteCacheDirectory))
        {
          vtkErrorMacro("Failed to initialize cache sentinel file in " << this->RemoteCacheDirectory << ".");
        }
      }
    }
    else if (!vtksys::SystemTools::FileIsDirectory(this->RemoteCacheDirectory.c_str()))
    {
      vtkErrorMacro("Cache path is not a directory: " << this->RemoteCacheDirectory << ".");
    }
    else if (!this->HasSentinelFileInDirectory(this->RemoteCacheDirectory))
    {
      if (this->IsDirectoryEmpty(this->RemoteCacheDirectory))
      {
        if (!this->CreateSentinelFileInDirectory(this->RemoteCacheDirectory))
        {
          vtkErrorMacro("Failed to initialize cache sentinel file in empty directory " << this->RemoteCacheDirectory << ".");
        }
      }
      else
      {
        vtkWarningMacro("Cache directory does not contain sentinel file " << this->GetSentinelFileName()
                                                                          << ". Destructive cache operations are disabled until the sentinel is created.");
      }
    }
  }

  this->UpdateCacheInformation();
}

//----------------------------------------------------------------------------
const char* vtkCacheManager::GetRemoteCacheDirectory()
{
  return (this->RemoteCacheDirectory.c_str());
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::GetSentinelFileName() const
{
  return std::string(".slicer-cache");
}

//----------------------------------------------------------------------------
bool vtkCacheManager::HasSentinelFile() const
{
  return this->HasSentinelFileInDirectory(this->RemoteCacheDirectory.c_str());
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CreateSentinelFile() const
{
  return this->CreateSentinelFileInDirectory(this->RemoteCacheDirectory.c_str());
}

//----------------------------------------------------------------------------
bool vtkCacheManager::IsRemoteReference(const char* uri)
{
  std::string uriString(uri);

  //--- get all characters up to (and not including) the '://'
  int index;
  if ((index = (int)(uriString.find("://", 0))) != (int)(std::string::npos))
  {
    std::string prefix = uriString.substr(0, index);
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ((index = (int)(prefix.find("]:", 0))) != (int)(std::string::npos))
    {
      //--- if so, strip off the bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr(index + 2);
    }
    if (prefix == "file")
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    vtkDebugMacro("URI " << uri << " contains no file:// or other prefix.");
    //--- doesn't seem to be a :// in the string.
    return false;
  }
}

//----------------------------------------------------------------------------
bool vtkCacheManager::IsLocalReference(const char* uri)
{
  int index;
  std::string uriString(uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ((index = (int)(uriString.find("://", 0))) != (int)(std::string::npos))
  {
    prefix = uriString.substr(0, index);
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ((index = (int)(prefix.find("]:", 0))) != (int)(std::string::npos))
    {
      //--- if so, strip off the bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr(index + 2);
    }
    if (prefix == "file")
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    vtkWarningMacro("URI " << uri << " contains no file:// or other prefix.");
    return false;
  }
}

//----------------------------------------------------------------------------
bool vtkCacheManager::LocalFileExists(const char* uri)
{
  int index;
  std::string uriString(uri);
  std::string filename;

  //--- get all characters up to (and not including) the '://'
  if ((index = (int)(uriString.find("://", 0))) != (int)(std::string::npos))
  {
    //--- is this the correct index???
    filename = uriString.substr(index + 3);
  }
  else
  {
    filename = uri;
  }

  if (vtksys::SystemTools::FileExists(filename.c_str()))
  {
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------
void vtkCacheManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "RemoteCacheDirectory: " << this->GetRemoteCacheDirectory() << "\n";
  os << indent << "RemoteCacheLimit: " << this->GetRemoteCacheLimit() << "\n";
  os << indent << "CurrentCacheSize: " << this->GetCurrentCacheSize() << "\n";
  os << indent << "EnableForceRedownload: " << this->GetEnableForceRedownload() << "\n";
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkCacheManager::GetCachedFiles() const
{
  std::vector<std::string> paths;
  paths.reserve(this->CacheEntries.size());
  for (const CacheEntry& e : this->CacheEntries)
  {
    paths.push_back(e.Path);
  }
  return paths;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkCacheManager::GetCachedFilesInDirectory(const std::string& dirname)
{
  std::vector<std::string> cachedFiles;
  std::vector<vtkCacheManagerCachedFile> cachedFileInfo;
  if (!this->CollectCachedFilesRecursively(dirname, dirname, cachedFileInfo))
  {
    vtkDebugMacro("vtkCacheManager::GetCachedFilesInDirectory: Directory " << dirname << " doesn't look like a directory.\n");
    return cachedFiles;
  }

  std::sort(cachedFileInfo.begin(),
            cachedFileInfo.end(),
            [](const vtkCacheManagerCachedFile& left, const vtkCacheManagerCachedFile& right)
            {
              if (left.ModifiedTime == right.ModifiedTime)
              {
                return left.Path < right.Path;
              }
              return left.ModifiedTime > right.ModifiedTime;
            });
  cachedFiles.reserve(cachedFileInfo.size());
  for (std::vector<vtkCacheManagerCachedFile>::const_iterator it = cachedFileInfo.begin(); it != cachedFileInfo.end(); ++it)
  {
    cachedFiles.push_back(it->Path);
  }

  return cachedFiles;
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::EncodeURI(const char* uri)
{
  if (uri == nullptr)
  {
    return std::string();
  }
  std::string kwInString = std::string(uri);
  // encode %
  vtksys::SystemTools::ReplaceString(kwInString, "%", "%25");
  // encode space
  vtksys::SystemTools::ReplaceString(kwInString, " ", "%20");
  // encode single quote
  vtksys::SystemTools::ReplaceString(kwInString, "'", "%27");
  // encode greater than
  vtksys::SystemTools::ReplaceString(kwInString, ">", "%3E");
  // encode less than
  vtksys::SystemTools::ReplaceString(kwInString, "<", "%3C");
  // encode double quote
  vtksys::SystemTools::ReplaceString(kwInString, "\"", "%22");

  return kwInString;
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::GetFilenameFromURI(const std::string& uri)
{
  return this->GetFilenameFromURI(uri.c_str());
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::GetFilenameFromURI(const char* uri)
{
  //--- this method should return the absolute path of the
  //--- file that a storage node will read from in cache
  //--- (the download destination file).

  if (uri == nullptr)
  {
    vtkDebugMacro("GetFilenameFromURI: input uri is null");
    return std::string();
  }

  const char* mapcheck = this->GetFileFromURIMap(uri);
  if (mapcheck != nullptr)
  {
    return (mapcheck);
  }

  std::string kwInString = std::string(uri);

  std::string::size_type loc = kwInString.find("?");
  if (loc != kwInString.npos)
  {
    kwInString = kwInString.substr(0, loc);
    vtkWarningMacro("Stripping question mark and trailing characters from uri." << kwInString.c_str());
  }
  //--- First decode special characters
  // decode double quote
  vtksys::SystemTools::ReplaceString(kwInString, "%22", "\"");
  // decode less than
  vtksys::SystemTools::ReplaceString(kwInString, "%3C", "<");
  // decode greater than
  vtksys::SystemTools::ReplaceString(kwInString, "%3E", ">");
  // decode single quote
  vtksys::SystemTools::ReplaceString(kwInString, "%27", "'");
  // decode space
  vtksys::SystemTools::ReplaceString(kwInString, "%20", " ");
  // decode %
  vtksys::SystemTools::ReplaceString(kwInString, "%25", "%");

  //--- Now strip off all the uri prefix (does this work?)

  std::string newFileName;
  std::string fileName;
  std::string baseName;
  std::string extensionName;
  std::string tmpName = vtksys::SystemTools::GetFilenameName(kwInString);
  std::string versionExtension;

  vtkDebugMacro("GetFilenameFromURI: got fileName name " << tmpName.c_str());

  //--- Now check for trailing version numbers.
  //  std::string::size_type found = tmpName.find_first_of ( ".");
  std::string::size_type found = tmpName.find_last_of(".");
  if (found != tmpName.npos)
  {
    //--- save the extension_version and baseName.
    versionExtension = tmpName.substr(found);
    baseName = tmpName.substr(0, found);

    found = versionExtension.find_first_of("_");
    if (found != versionExtension.npos)
    {
      //--- found a version number modifying the extension.
      extensionName = versionExtension.substr(0, found);
    }
    else
    {
      extensionName = versionExtension;
    }
  }
  if (baseName.c_str() != nullptr)
  {
    newFileName = baseName.c_str();
  }
  if (extensionName.c_str() != nullptr)
  {
    newFileName += extensionName.c_str();
  }

  //--- Create absolute path
  if (this->GetRemoteCacheDirectory() == nullptr || //
      strcmp(this->GetRemoteCacheDirectory(), "") == 0)
  {
    vtkErrorMacro("GetFilenameFromURI: remote cache dir is not set! The file will appear in the current working dir.");
  }
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath(this->GetRemoteCacheDirectory(), pathComponents);
  pathComponents.push_back(newFileName);
  fileName = vtksys::SystemTools::JoinPath(pathComponents);

  vtkDebugMacro("GetFilenameFromURI: returning " << fileName.c_str());
  return fileName;
}

//----------------------------------------------------------------------------
void vtkCacheManager::UpdateCacheInformation()
{
  if (this->UpdatingCacheInformation)
  {
    return;
  }
  this->UpdatingCacheInformation = true;

  // Scan the top-level cache directory entries.
  std::vector<CacheEntry> newEntries;
  if (!this->RemoteCacheDirectory.empty())
  {
    const std::string canonicalCacheDirectory = this->CanonicalizePath(this->GetRemoteCacheDirectory());
    vtksys::Directory cacheDirectory;
    if (cacheDirectory.Load(this->GetRemoteCacheDirectory()))
    {
      newEntries.reserve(cacheDirectory.GetNumberOfFiles());
      for (size_t fileNum = 0; fileNum < cacheDirectory.GetNumberOfFiles(); ++fileNum)
      {
        const char* fileName = cacheDirectory.GetFile(static_cast<unsigned long>(fileNum));
        if (IsDirectoryNavigationEntry(fileName) || this->GetSentinelFileName() == fileName)
        {
          continue;
        }

        CacheEntry entry;
        entry.Path = this->JoinPath(this->GetRemoteCacheDirectory(), fileName);
        entry.SizeBytes = 0;
        entry.ModifiedTime = 0;
        entry.IsDirectory = vtksys::SystemTools::FileIsDirectory(entry.Path.c_str());
        entry.FileCount = 0;
        entry.ExceedsCacheSize = false;

        std::string canonicalTargetPath = this->CanonicalizePath(entry.Path.c_str());
        if (!canonicalTargetPath.empty() && !canonicalCacheDirectory.empty() && !this->PathIsWithinDirectory(canonicalCacheDirectory, canonicalTargetPath))
        {
          vtkWarningMacro("Skipping cache entry outside cache root: " << entry.Path);
          continue;
        }

        if (entry.IsDirectory)
        {
          std::vector<vtkCacheManagerCachedFile> cachedFilesInDirectory;
          if (!this->CollectCachedFilesRecursively(entry.Path, entry.Path, cachedFilesInDirectory))
          {
            vtkWarningMacro("UpdateCacheInformation could not inspect directory: " << entry.Path);
            continue;
          }
          for (const vtkCacheManagerCachedFile& cachedFile : cachedFilesInDirectory)
          {
            entry.SizeBytes += static_cast<unsigned long long>(vtksys::SystemTools::FileLength(cachedFile.Path.c_str()));
            entry.ModifiedTime = std::max(entry.ModifiedTime, cachedFile.ModifiedTime);
            entry.FileCount++;
          }
          if (entry.FileCount == 0)
          {
            entry.ModifiedTime = static_cast<long long>(vtksys::SystemTools::ModifiedTime(entry.Path));
          }
        }
        else
        {
          entry.SizeBytes = static_cast<unsigned long long>(vtksys::SystemTools::FileLength(entry.Path.c_str()));
          entry.ModifiedTime = static_cast<long long>(vtksys::SystemTools::ModifiedTime(entry.Path));
        }

        newEntries.push_back(entry);
      }
    }
  }

  // Compute total size in MB.
  unsigned long long cacheSizeBytes = 0;
  for (const CacheEntry& e : newEntries)
  {
    cacheSizeBytes += e.SizeBytes;
  }
  float newCacheSize = static_cast<float>(cacheSizeBytes / MB);

  // Mark entries that PruneCache() would evict: oldest-first until within limit.
  const unsigned long long cacheLimitBytes = static_cast<unsigned long long>(this->GetRemoteCacheLimit()) * static_cast<unsigned long long>(MB);
  if (cacheSizeBytes > cacheLimitBytes)
  {
    std::vector<size_t> byAge(newEntries.size());
    std::iota(byAge.begin(), byAge.end(), 0);
    std::sort(byAge.begin(),
              byAge.end(),
              [&newEntries](size_t a, size_t b)
              {
                if (newEntries[a].ModifiedTime == newEntries[b].ModifiedTime)
                {
                  return newEntries[a].Path < newEntries[b].Path;
                }
                return newEntries[a].ModifiedTime < newEntries[b].ModifiedTime;
              });
    unsigned long long remaining = cacheSizeBytes;
    for (size_t idx : byAge)
    {
      if (remaining <= cacheLimitBytes)
      {
        break;
      }
      newEntries[idx].ExceedsCacheSize = true;
      remaining = (newEntries[idx].SizeBytes < remaining) ? remaining - newEntries[idx].SizeBytes : 0;
    }
  }

  // Sort newest-first for consumers.
  std::sort(newEntries.begin(),
            newEntries.end(),
            [](const CacheEntry& a, const CacheEntry& b)
            {
              if (a.ModifiedTime == b.ModifiedTime)
              {
                return a.Path < b.Path;
              }
              return a.ModifiedTime > b.ModifiedTime;
            });

  // Detect changes by comparing sorted entry paths and total size.
  std::vector<std::string> oldPaths, newPaths;
  for (const CacheEntry& e : this->CacheEntries)
  {
    oldPaths.push_back(e.Path);
  }
  for (const CacheEntry& e : newEntries)
  {
    newPaths.push_back(e.Path);
  }
  const bool changed = (oldPaths != newPaths || newCacheSize != this->CurrentCacheSize);

  this->CacheEntries = std::move(newEntries);
  this->CurrentCacheSize = newCacheSize;

  if (this->CurrentCacheSize > static_cast<float>(this->RemoteCacheLimit))
  {
    this->InvokeEvent(vtkCacheManager::CacheLimitExceededEvent);
  }

  if (changed)
  {
    this->Modified();
  }

  this->UpdatingCacheInformation = false;
}

//----------------------------------------------------------------------------
void vtkCacheManager::DeleteFromCache(const std::string& filename)
{
  this->DeleteFromCache(filename.c_str());
}

//----------------------------------------------------------------------------
void vtkCacheManager::DeleteFromCache(const char* target)
{
  if (target == nullptr)
  {
    return;
  }

  //--- discover if target already has Remote Cache Directory prepended to path.
  //--- if not, put it there.

  std::string foundPath = this->FindCachedFile(target, this->GetRemoteCacheDirectory());
  if (foundPath.empty())
  {
    vtkDebugMacro("RemoveFromCache: can't find the target file " << target << ", so there's nothing to do, returning.");
    return;
  }

  std::string reason;
  if (!this->CanDeletePathInCache(this->GetRemoteCacheDirectory(), foundPath.c_str(), reason))
  {
    vtkWarningMacro("DeleteFromCache was blocked for safety (" << reason << "): " << foundPath);
    return;
  }

  this->MarkNodesBeforeDeletingDataFromCache(target);

  //--- remove the file or directory in str....
  vtkDebugMacro("Removing " << foundPath << " from disk and from record of cached files.");
  bool deleted = false;
  if (vtksys::SystemTools::FileIsDirectory(foundPath))
  {
    deleted = vtksys::SystemTools::RemoveADirectory(foundPath).IsSuccess();
    if (!deleted)
    {
      vtkWarningMacro("Unable to remove cached directory " << foundPath << " from disk.");
    }
  }
  else
  {
    deleted = vtksys::SystemTools::RemoveFile(foundPath).IsSuccess();
    if (!deleted)
    {
      vtkWarningMacro("Unable to remove cached file " << foundPath << " from disk.");
    }
  }

  if (deleted)
  {
    this->UpdateCacheInformation();
    this->InvokeEvent(vtkCacheManager::CacheDeleteEvent);
  }
}

//----------------------------------------------------------------------------
bool vtkCacheManager::ClearCache()
{
  //--- Careful! Before making this call, prompt user
  //--- with the RemoteCacheDirectory name and
  //--- ask for confirmation whether to delete the
  //--- directory and all of its contents...
  //--- Removes the CacheDirectory all together
  //--- and then creates the directory again.
  if (this->RemoteCacheDirectory.empty())
  {
    return true;
  }

  if (!this->HasSentinelFile())
  {
    vtkWarningMacro("ClearCache was blocked, as sentinel file was not found: " << this->GetSentinelFilePath(this->RemoteCacheDirectory.c_str()));
    return false;
  }

  this->MarkNodesBeforeDeletingDataFromCache(this->RemoteCacheDirectory.c_str());

  if (!vtksys::SystemTools::RemoveADirectory(this->RemoteCacheDirectory.c_str()))
  {
    vtkWarningMacro("Cache cleared: Error: unable to remove cache directory and its contents.");
    return false;
  }
  if (!vtksys::SystemTools::MakeDirectory(this->RemoteCacheDirectory.c_str()))
  {
    vtkWarningMacro("Cache cleared: Error: unable to recreate cache directory after deleting its contents.");
    return false;
  }
  if (!this->CreateSentinelFileInDirectory(this->RemoteCacheDirectory.c_str()))
  {
    vtkWarningMacro("Cache cleared: Error: unable to recreate cache sentinel file.");
    return false;
  }

  this->UpdateCacheInformation();
  this->InvokeEvent(vtkCacheManager::CacheClearEvent);
  return true;
}

//----------------------------------------------------------------------------
float vtkCacheManager::GetCurrentCacheSize()
{
  return this->CurrentCacheSize;
}

//----------------------------------------------------------------------------
void vtkCacheManager::MarkNode(std::string nodeStoragePath)
{
  //--- Find the MRML node that points to this file in cache.
  //--- If such a node exists, mark it as modified since read,
  //--- so that a user will be prompted to save the
  //--- data elsewhere (since it'll be deleted from cache.)
  if (this->MRMLScene == nullptr)
  {
    return;
  }
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLStorableNode");
  for (int n = 0; n < nnodes; n++)
  {
    vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLStorableNode"));
    if (!node)
    {
      continue;
    }
    int numStorageNodes = node->GetNumberOfStorageNodes();
    for (int i = 0; i < numStorageNodes; i++)
    {
      if (!node->GetNthStorageNode(i))
      {
        continue;
      }
      std::string uri = node->GetNthStorageNode(i)->GetFullNameFromFileName();
      if (nodeStoragePath == uri)
      {
        node->GetNthStorageNode(i)->InvalidateFile();
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkCacheManager::MarkNodesBeforeDeletingDataFromCache(const char* target)
{
  //--- Target is a file or directory marked for deletion in cache.
  //--- If target is a file, this method finds any node holding a
  //--- reference to it and marks that node as ModifiedSinceRead.
  //--- If target is a directory, the method traverses the directory
  //--- and any subdirectories, and marks nodes holding references
  //--- to any of the files within as ModifiedSinceRead.
  //--- might be a slow performer....(?)

  if (!target)
  {
    return;
  }

  if (vtksys::SystemTools::FileIsDirectory(target))
  {
    vtkDebugMacro("MarkNodesBeforeDeletingDataFromCache: target is a directory: " << target);
    std::vector<vtkCacheManagerCachedFile> cachedFiles;
    if (!this->CollectCachedFilesRecursively(target, target, cachedFiles))
    {
      return;
    }
    for (std::vector<vtkCacheManagerCachedFile>::const_iterator it = cachedFiles.begin(); it != cachedFiles.end(); ++it)
    {
      this->MarkNode(it->Path);
    }
  }
  else
  {
    if (vtksys::SystemTools::FileExists(target))
    {
      this->MarkNode(target);
    }
  }
}

//----------------------------------------------------------------------------
float vtkCacheManager::GetFreeCacheSpaceRemaining()
{
  float diff = (float(this->RemoteCacheLimit) - this->CurrentCacheSize);
  return (diff);
}

//----------------------------------------------------------------------------
std::vector<vtkCacheManager::CacheEntry> vtkCacheManager::GetCacheEntries() const
{
  return this->CacheEntries;
}

//----------------------------------------------------------------------------
bool vtkCacheManager::PruneCache()
{
  if (this->RemoteCacheDirectory.empty())
  {
    return false;
  }

  if (!this->HasSentinelFile())
  {
    vtkWarningMacro("Cache pruning is disabled as no sentinel file is found in the cache directory.");
    return false;
  }

  const unsigned long long cacheLimitBytes = static_cast<unsigned long long>(this->GetRemoteCacheLimit()) * static_cast<unsigned long long>(MB);
  const std::string canonicalCacheDirectory = this->CanonicalizePath(this->GetRemoteCacheDirectory());
  if (canonicalCacheDirectory.empty())
  {
    vtkWarningMacro("Cache pruning is disabled as cache directory could not be canonicalized: " << this->GetRemoteCacheDirectory());
    return false;
  }

  std::vector<CacheEntry> pruneTargets = this->GetCacheEntries();

  unsigned long long totalSizeBytes = 0;
  for (const CacheEntry& entry : pruneTargets)
  {
    totalSizeBytes += entry.SizeBytes;
  }

  if (totalSizeBytes <= cacheLimitBytes)
  {
    // Cache is already within limit, no need to prune, but refresh
    // cached bookkeeping so callers observe up-to-date cache usage.
    this->UpdateCacheInformation();
    return true;
  }

  // We will evict cache entries starting with the oldest modified time
  std::sort(pruneTargets.begin(),
            pruneTargets.end(),
            [](const CacheEntry& left, const CacheEntry& right)
            {
              if (left.ModifiedTime == right.ModifiedTime)
              {
                return left.Path < right.Path;
              }
              return left.ModifiedTime < right.ModifiedTime;
            });

  bool wasModified = false;
  for (const CacheEntry& entry : pruneTargets)
  {
    if (totalSizeBytes <= cacheLimitBytes)
    {
      break;
    }

    std::string reason;
    if (!this->CanDeletePathInCache(this->GetRemoteCacheDirectory(), entry.Path.c_str(), reason))
    {
      vtkWarningMacro("PruneCache was blocked for safety (" << reason << "): " << entry.Path);
      continue;
    }

    this->MarkNodesBeforeDeletingDataFromCache(entry.Path.c_str());

    bool deleted = false;
    if (entry.IsDirectory)
    {
      deleted = vtksys::SystemTools::RemoveADirectory(entry.Path).IsSuccess();
    }
    else
    {
      deleted = vtksys::SystemTools::RemoveFile(entry.Path).IsSuccess();
    }

    if (!deleted)
    {
      vtkWarningMacro("Failed to evict cached " << (entry.IsDirectory ? "directory " : "file ") << entry.Path << ".");
      continue;
    }

    wasModified = true;
    if (entry.SizeBytes > totalSizeBytes)
    {
      totalSizeBytes = 0;
    }
    else
    {
      totalSizeBytes -= entry.SizeBytes;
    }
  }

  if (wasModified)
  {
    this->UpdateCacheInformation();
    this->InvokeEvent(vtkCacheManager::CacheDeleteEvent);
  }

  if (totalSizeBytes > cacheLimitBytes)
  {
    vtkWarningMacro("PruneCache could not bring cache size within limit. Remaining bytes: " << totalSizeBytes << ", limit bytes: " << cacheLimitBytes);
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CachedFileExists(const std::string& filename)
{
  return this->CachedFileExists(filename.c_str());
}

//----------------------------------------------------------------------------
bool vtkCacheManager::CachedFileExists(const char* filename)
{
  if (IsNullOrEmpty(filename))
  {
    return false;
  }

  if (vtksys::SystemTools::FileExists(filename))
  {
    return true;
  }

  std::string testFile = this->JoinPath(this->RemoteCacheDirectory, filename);
  if (vtksys::SystemTools::FileExists(testFile.c_str()))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
std::string vtkCacheManager::FindCachedFile(const char* target, const char* dirname)
{
  if (target == nullptr || dirname == nullptr)
  {
    vtkErrorMacro("FindCachedFile: target or dirname null");
    return "";
  }

  if (vtksys::SystemTools::FileIsDirectory(dirname))
  {
    vtkDebugMacro("FindCachedFile: dirname is a directory: " << dirname);
    vtksys::Directory dir;
    dir.Load(dirname);
    size_t fileNum;

    //--- get files in cache dir and add to vector of strings.
    for (fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum)
    {
      const char* fileName = dir.GetFile(static_cast<unsigned long>(fileNum));
      if (!IsDirectoryNavigationEntry(fileName))
      {
        std::string testFile = fileName;
        //--- Check for match to target
        //--- does the file or directory match the target?
        if (!strcmp(target, testFile.c_str()))
        {
          return this->JoinPath(dirname, testFile);
        }

        //--- does the file or directory match the target with full path?
        std::string fullName = this->JoinPath(dirname, testFile);
        if (!strcmp(target, fullName.c_str()))
        {
          return fullName;
        }

        //--- if no match, and the file is a directory, go inside and
        //--- do some recursive thing to add those files to cached list
        if (vtksys::SystemTools::FileIsDirectory(fullName.c_str()))
        {
          ///---compute dir and filename for recursive hunt
          std::string result = this->FindCachedFile(target, fullName.c_str());
          if (!result.empty())
          {
            return result;
          }
        }
      }
    }
  }
  else
  {
    vtkDebugMacro("FindCachedFile: Directory " << dirname << " doesn't look like a directory. \n");
    return "";
  }

  return "";
}
