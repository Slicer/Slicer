#ifndef __vtkCacheManager_h
#define __vtkCacheManager_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLScene.h"
class vtkCallbackCommand;

// VTK includes
#include <vtkObject.h>
#include <vtkWeakPointer.h>

// STD includes
#include <string>
#include <vector>
#include <map>

#ifndef vtkObjectPointer
# define vtkObjectPointer(xx) (reinterpret_cast<vtkObject**>((xx)))
#endif

/// \brief Manages the on-disk remote cache used by MRML storage.
///
/// The class manages a folder that stores temporary files, mostly used for local caching of remote data.
///
/// The cache folder needs to be cleaned when the maximum size is reached. To prevent accidental deletion of
/// non-cache user data, the class uses a "sentinel file" mechanism. Presence of a sentinel file (.slicer-cache)
/// in the folder marks it as owned by the cache manager, allowing the cache manager to delete files in it.
/// Deletion is strictly limited to files within the cache directory (symlinks are resolved and file is only
/// deleted if the canonicalized path is inside the cache root folder).
///
class VTK_MRML_EXPORT vtkCacheManager : public vtkObject
{
public:
  /// The Usual vtk class functions
  static vtkCacheManager* New();
  vtkTypeMacro(vtkCacheManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetMRMLScene(vtkMRMLScene* scene) { this->MRMLScene = scene; }

  ///
  /// Sets the name of the directory to use for local file caching
  /// Does some checking to make sure this is a valid directory
  /// on the local system. Makes sure there's NO "/" at the end
  /// of the string, or kwsys/SystemTools will not see as a valid dir.
  virtual void SetRemoteCacheDirectory(const char* dir);
  virtual void SetRemoteCacheDirectory(const std::string& dir) { this->SetRemoteCacheDirectory(dir.c_str()); }

  ///
  /// Returns the name of the directory to use for local file caching
  const char* GetRemoteCacheDirectory();

  /// Cache size limit in MB.
  vtkGetMacro(RemoteCacheLimit, int);
  vtkSetMacro(RemoteCacheLimit, int);

  ///
  /// Called when a file is loaded or removed from the cache.
  void UpdateCacheInformation();

  /// Current cache size in MB. Updated by UpdateCacheInformation().
  float GetCurrentCacheSize();
  /// Returns the amount of free space remaining in the cache directory, in MB. Updated by UpdateCacheInformation().
  float GetFreeCacheSpaceRemaining();
  /// Get list of cached files. Updated by UpdateCacheInformation().
  /// Get files in the folder, ordered by modification time from newest to oldest.
  /// Does not include the sentinel file.
  std::vector<std::string> GetCachedFiles() const;

  /// Describes a top-level item (file or folder) in the cache directory.
  struct CacheEntry
  {
    std::string Path;
    unsigned long long SizeBytes;
    long long ModifiedTime;
    bool IsDirectory;
    int FileCount;         ///< number of files contained in the directory (0 for file entries)
    bool ExceedsCacheSize; ///< true if PruneCache() would remove this entry to bring the cache within its size limit
  };

  /// Returns one CacheEntry per top-level item in the cache directory.
  /// Updated by UpdateCacheInformation()
  /// Entries are sorted newest-first (descending ModifiedTime).
  /// Does not include the sentinel file.
  /// CacheEntry::ExceedsCacheSize is set for entries that PruneCache() would evict.
  std::vector<CacheEntry> GetCacheEntries() const;

  /// Delete oldest files until on-disk cache usage fits within RemoteCacheLimit.
  /// Returns true on success and false on failure.
  bool PruneCache();

  ///
  /// Removes all files from the cachedir
  bool ClearCache();

  /// Remove a target directory or file from the cache folder.
  /// If the target is a directory, it will be removed recursively.
  /// Before deletion, any MRML nodes that hold the URI as a reference will be marked as "modified since read".
  void DeleteFromCache(const char* target);
  void DeleteFromCache(const std::string& filename);

  ///
  /// Checks to see if a URI appears to point to remote location
  /// and returns true if so. Looks for a '://' and if present,
  /// checks to see if the prefix is 'file'. If not 'file' but the
  /// thing:/// pattern exists, then returns true.
  virtual bool IsRemoteReference(const char* uri);

  ///
  /// Looks for a 'file://' in the URI and if present, returns true.
  virtual bool IsLocalReference(const char* uri);

  ///
  /// Checks to see if a URI is a file on disk and returns
  /// true if so. Strips off a file:/// prefix if present, and
  /// expects an absolute path.
  virtual bool LocalFileExists(const char* uri);

  ///
  /// Takes a filename and a dirname (usually called with the
  /// RemoteCachedDirectory) and returns the full path of
  /// the filename if it exists under the dirname.
  std::string FindCachedFile(const char* target, const char* dirname);

  ///
  /// Checks to see if the The URI provided exists on disk.
  /// If not, it appends the Remote Cache Directory path
  /// and checks again, in case no path was provided.
  /// If neither exists, returns false. If one exists, returns true.
  virtual bool CachedFileExists(const char* filename);
  virtual bool CachedFileExists(const std::string& filename);

  ///
  /// Extracts the filename from the URI and prepends the
  /// Remote Cache Directory path to it. Returns the full path.
  /// NOTE: this method looks at a filename's extension and
  /// if appended version numbers have been added, it attempts
  /// to strip them out of the extension and add them to the
  /// filenamebase. So filename.nrrd_010 would become filename.nrrd.
  /// This will cause problems for any file type with an '_' in its extension.
  std::string GetFilenameFromURI(const char* uri);
  std::string GetFilenameFromURI(const std::string& uri);
  std::string EncodeURI(const char* uri);

  /// Sentinel file name used to identify cache directories where file deletion is allowed.
  std::string GetSentinelFileName() const;
  /// True if the sentinel file exists in the current cache directory.
  bool HasSentinelFile() const;
  /// Creates the sentinel file in the current cache directory.
  bool CreateSentinelFile() const;

  /// Returns true if the specified directory is empty.
  bool IsDirectoryEmpty(const std::string& directoryPath) const;
  /// True if the sentinel file exists in the specified directory.
  bool HasSentinelFileInDirectory(const std::string& directoryPath) const;
  /// Creates the sentinel file in the specified directory.
  bool CreateSentinelFileInDirectory(const std::string& directoryPath) const;
  /// Get files in the folder, ordered by modification time from newest to oldest.
  /// Does not include the sentinel file.
  std::vector<std::string> GetCachedFilesInDirectory(const std::string& directoryPath);

  vtkGetMacro(EnableForceRedownload, int);
  vtkSetMacro(EnableForceRedownload, int);

  vtkGetMacro(InsufficientFreeBufferNotificationFlag, int);
  vtkSetMacro(InsufficientFreeBufferNotificationFlag, int);

  void MapFileToURI(const char* uri, const char* fname);

  /// Currently these are not used.
  enum
  {
    NoCachedFile = 0,
    OldCachedFile,
    CachedFile
  };

  enum
  {
    InsufficientFreeBufferEvent = 21000,
    CacheLimitExceededEvent,
    CacheDeleteEvent,
    CacheDirtyEvent,
    CacheClearEvent
  };

  const char* GetFileFromURIMap(const char* uri);

private:
  struct vtkCacheManagerCachedFile
  {
    std::string Path;
    long long ModifiedTime;
  };

  vtkWeakPointer<vtkMRMLScene> MRMLScene;
  std::string RemoteCacheDirectory;
  std::map<std::string, std::string> UriMap;
  int InsufficientFreeBufferNotificationFlag;
  int RemoteCacheLimit;
  int EnableForceRedownload;

  std::vector<CacheEntry> CacheEntries;
  float CurrentCacheSize;
  bool UpdatingCacheInformation;

  bool CollectCachedFilesRecursively(const std::string& directoryPath, const std::string& rootDirectoryPath, std::vector<vtkCacheManagerCachedFile>& cachedFiles) const;

  /// Before a file or directory is deleted,
  /// Marks any nodes that hold the URI as
  /// a reference as modified since read.
  void MarkNodesBeforeDeletingDataFromCache(const char*);

  void MarkNode(std::string);

  std::string CanonicalizePath(const char* path) const;
  std::string JoinPath(const std::string& directoryPath, const std::string& fileName) const;
  bool PathIsWithinDirectory(const std::string& directoryPath, const std::string& path) const;

  std::string GetSentinelFilePath(const char* cacheDirectory) const;
  std::string GetSentinelFilePath(const std::string& cacheDirectory) const;

  bool CanDeletePathInCache(const char* cacheDirectory, const char* pathToDelete, std::string& reason) const;

protected:
  vtkCacheManager();
  ~vtkCacheManager() override;
  vtkCacheManager(const vtkCacheManager&);
  void operator=(const vtkCacheManager&);

  ///
  /// Holder for callback
  vtkCallbackCommand* CallbackCommand;
};

#endif
