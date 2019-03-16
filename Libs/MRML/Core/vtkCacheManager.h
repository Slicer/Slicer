#ifndef __vtkCacheManager_h
#define __vtkCacheManager_h

// MRML includes
#include "vtkMRML.h"
class vtkCallbackCommand;
class vtkMRMLScene;

// VTK includes
#include <vtkObject.h>

// STD includes
#include <string>
#include <vector>
#include <map>

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

class VTK_MRML_EXPORT vtkCacheManager : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkCacheManager *New();
  vtkTypeMacro(vtkCacheManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkGetMacro (InsufficientFreeBufferNotificationFlag, int );
  vtkSetMacro (InsufficientFreeBufferNotificationFlag, int );

  ///
  /// Sets the name of the directory to use for local file caching
  /// Does some checking to make sure this is a valid directory
  /// on the local system. Makes sure there's NO "/" at the end
  /// of the string, or kwsys/SystemTools will not see as a valid dir.
  virtual void SetRemoteCacheDirectory (const char *dir );

  ///
  /// Returns the name of the directory to use for local file caching
  const char *GetRemoteCacheDirectory ();

  ///
  /// Called when a file is loaded or removed from the cache.
  void UpdateCacheInformation ( );
  ///
  /// Removes a target from the list of locally cached files and directories
  void DeleteFromCachedFileList ( const char * target );

  //Description:
  /// Remove a target directory or file from the cache.
  void DeleteFromCache( const char *target );

  ///
  /// Removes all files from the cachedir
  /// and removes all filenames from CachedFileList
  int ClearCache ( );
  /// This method is called after ClearCache(),
  /// to see if that method actually cleaned the cache.
  /// If not, an event (CacheDirtyEvent) is invoked.
  int ClearCacheCheck ( );

  ///
  /// Before a file or directory is deleted,
  /// Marks any nodes that hold the uri as
  /// a reference as modified since read.
  void MarkNodesBeforeDeletingDataFromCache (const char *);

  ///
  /// Checks to see if a uri appears to point to remote location
  /// and returns true if so. Looks for a '://' and if present,
  /// checks to see if the prefix is 'file'. If not 'file' but the
  /// thing:/// pattern exists, then returns true.
  virtual int IsRemoteReference ( const char *uri );
  ///
  /// Looks for a 'file://' in the uri and if present, returns true.
  virtual int IsLocalReference ( const char *uri );

  ///
  /// Checks to see if a uri is a file on disk and returns
  /// true if so. Strips off a file:/// prefix if present, and
  /// expects an absolute path.
  virtual int LocalFileExists ( const char *uri );

  ///
  /// Takes a filename and a dirname (usually called with the
  /// RemoteCachedDirectory) and returns the full path of
  /// the filename if it exists under the dirname.
  const char* FindCachedFile ( const char * target, const char *dirname );

  ///
  /// Checks to see if the The uri provided exists on disk.
  /// If not, it appends the Remote Cache Directory path
  /// and checks again, in case no path was provided.
  /// If neither exists, returns 0. If one exists, returns 1.
  virtual int CachedFileExists ( const char *filename );

  ///
  /// Extracts the filename from the URI and prepends the
  /// Remote Cache Directory path to it. Returns the full path.
  /// NOTE: this method looks at a filename's extension and
  /// if appended version numbers have been added, it attempts
  /// to strip them out of the extension and add them to the
  /// filenamebase. So filename.nrrd_010 would become filename.nrrd.
  /// This will cause problems for any file type with an '_' in its extension.
  const char* GetFilenameFromURI ( const char *uri );
  const char* AddCachePathToFilename ( const char *filename );
  const char* EncodeURI ( const char *uri );

  void CacheSizeCheck();
  void FreeCacheBufferCheck();
  float ComputeCacheSize( const char *dirname, unsigned long size );
  float GetCurrentCacheSize();
  float GetFreeCacheSpaceRemaining();

  std::vector< std::string > GetCachedFiles()const;

  ///
  vtkGetMacro ( RemoteCacheLimit, int );
  vtkSetMacro ( RemoteCacheLimit, int );
  vtkSetMacro ( CurrentCacheSize, float );
  vtkGetMacro ( RemoteCacheFreeBufferSize, int );
  vtkSetMacro ( RemoteCacheFreeBufferSize, int );
  vtkGetMacro ( EnableForceRedownload, int );
  vtkSetMacro ( EnableForceRedownload, int );
  //vtkGetMacro ( EnableRemoteCacheOverwriting, int );
  //vtkSetMacro ( EnableRemoteCacheOverwriting, int );
  void SetMRMLScene ( vtkMRMLScene *scene )
      {
      this->MRMLScene = scene;
      }
  void MapFileToURI ( const char *uri, const char *fname );

  void MarkNode ( std::string );
  /// in case we need these.
  enum
    {
      NoCachedFile=0,
      OldCachedFile,
      CachedFile
    };

  /// in case we need these.
  enum
    {
      InsufficientFreeBufferEvent =  21000,
      CacheLimitExceededEvent,
      CacheDeleteEvent,
      CacheDirtyEvent,
      CacheClearEvent
    };

  std::map<std::string, std::string> uriMap;
  const char *GetFileFromURIMap (const char *uri );

 private:
  int InsufficientFreeBufferNotificationFlag;
  int RemoteCacheLimit;
  float CurrentCacheSize;
  int RemoteCacheFreeBufferSize;
  int EnableForceRedownload;
  //int EnableRemoteCacheOverwriting;
  vtkMRMLScene *MRMLScene;

  std::string RemoteCacheDirectory;
  int GetCachedFileList(const char *dirname);
  std::vector< std::string > GetAllCachedFiles();
  /// This array contains a list of cached file names (without paths)
  /// in case it's faster to search thru this list than to
  /// snuffle thru a large cache dir. Must keep current
  /// with every download, remove from cache, and clearcache call.
  std::vector< std::string > CachedFileList;

 protected:
  vtkCacheManager();
  ~vtkCacheManager() override;
  vtkCacheManager(const vtkCacheManager&);
  void operator=(const vtkCacheManager&);

  ///
  /// Holder for callback
  vtkCallbackCommand *CallbackCommand;

};

#endif

