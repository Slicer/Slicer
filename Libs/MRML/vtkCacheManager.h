#ifndef __vtkCacheManager_h
#define __vtkCacheManager_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"
#include "vtksys/SystemTools.hxx"
#include "vtksys/RegularExpression.hxx"

#include "vtkMRML.h"


#include <string>
#include <vector>
#include <iterator>

class vtkCallbackCommand;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif


class VTK_MRML_EXPORT vtkCacheManager : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkCacheManager *New();
  vtkTypeRevisionMacro(vtkCacheManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the name of the directory to use for local file caching
  // Does some checking to make sure this is a valid directory
  // on the local system. Makes sure there's NO "/" at the end
  // of the string, or kwsys/SystemTools will not see as a valid dir.
  virtual void SetRemoteCacheDirectory (const char *dir );

  // Description:
  // Returns the name of the directory to use for local file caching
  const char *GetRemoteCacheDirectory () {
  return ( this->RemoteCacheDirectory.c_str() );
  };

  // Description:
  // Called when a file is loaded or removed from the cache.
  void UpdateCacheInformation ( );
  // Description:
  // Removes a target from the list of locally cached files and directories
  void DeleteFromCachedFileList ( const char * target );

  //Description:
  // Remove a target directory or file from the cache.
  void DeleteFromCache( const char *target );

  // Description:
  // Removes all files from the cachedir
  // and removes all filenames from CachedFileList
  int ClearCache ( );
  
  // Description:
  // Checks to see if a uri appears to point to remote location
  // and returns true if so. Looks for a '://' and if present,
  // checks to see if the prefix is 'file'. If not 'file' but the
  // thing:// pattern exists, then returns true.
  virtual int IsRemoteReference ( const char *uri );
  // Description:
  // Looks for a 'file://' in the uri and if present, returns true.
  virtual int IsLocalReference ( const char *uri );

  // Description:
  // Checks to see if a uri is a file on disk and returns 
  // true if so. Strips off a file:// prefix if present, and
  // expects an absolute path.
  virtual int LocalFileExists ( const char *uri );
    
  // Description:
  // Takes a filename and a dirname (usually called with the
  // RemoteCachedDirectory) and returns the full path of
  // the filename if it exists under the dirname.
  const char* FindCachedFile ( const char * target, const char *dirname );

  // Description:
  // Checks to see if the The uri provided exists on disk.
  // If not, it appends the Remote Cache Directory path
  // and checks again, in case no path was provided.
  // If neither exists, returns 0. If one exists, returns 1.
  virtual int CachedFileExists ( const char *filename );

  // Description:
  // Extracts the filename from the URI and prepends the
  // Remote Cache Directory path to it. Returns the full path.
  const char* GetFilenameFromURI ( const char *uri );
  const char* AddCachePathToFilename ( const char *filename );
  const char* EncodeURI ( const char *uri );
    
  void CacheSizeCheck();
  void FreeCacheBufferCheck();
  int GetFreeCacheSpaceRemaining();
  int GetCachedFileList(const char *dirname);
  unsigned long ComputeCacheSize( const char *dirname, unsigned long size );
  
  // Description:
  vtkGetMacro ( RemoteCacheLimit, int );
  vtkSetMacro ( RemoteCacheLimit, int );
  vtkGetMacro ( CurrentCacheSize, int );
  vtkSetMacro ( CurrentCacheSize, int );
  vtkGetMacro ( RemoteCacheFreeBufferSize, int );
  vtkSetMacro ( RemoteCacheFreeBufferSize, int );
  vtkGetMacro ( EnableForceRedownload, int );
  vtkSetMacro ( EnableForceRedownload, int );
  //vtkGetMacro ( EnableRemoteCacheOverwriting, int );
  //vtkSetMacro ( EnableRemoteCacheOverwriting, int );

  //BTX
  // in case we need these.
  enum
    {
      NoCachedFile=0,
      OldCachedFile,
      CachedFile,
    };

  // in case we need these.
  enum
    {
      InsufficientFreeBufferEvent =  21000,
      CacheLimitExceededEvent,
      CacheDeleteEvent,
      CacheClearEvent,
    };
  //ETX
  
 private:
  int RemoteCacheLimit;
  int CurrentCacheSize;
  int RemoteCacheFreeBufferSize;
  int EnableForceRedownload;
  //int EnableRemoteCacheOverwriting;


  //BTX
  std::string RemoteCacheDirectory;
  std::vector< std::string > GetAllCachedFiles();
  // This array contains a list of cached file names (without paths)
  // in case it's faster to search thru this list than to
  // snuffle thru a large cache dir. Must keep current
  // with every download, remove from cache, and clearcache call.
  std::vector< std::string > CachedFileList;
  //ETX

 protected:
  vtkCacheManager();
  virtual ~vtkCacheManager();
  vtkCacheManager(const vtkCacheManager&);
  void operator=(const vtkCacheManager&);

  // Description:
  // Holder for callback
  vtkCallbackCommand *CallbackCommand;

};

#endif

