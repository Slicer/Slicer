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
  void SetRemoteCacheDirectory (const char *dir ) {
  this->RemoteCacheDirectory = vtksys_stl::string (dir);
  };

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
  void RemoveFromCachedFileList ( const char * target );

  //Description:
  // Remove a target directory or file from the cache.
  void RemoveFromCache( const char *target );

  // Description:
  // Removes all files from the cachedir
  // and removes all filenames from CachedFileList
  void ClearCache ( );
  
  // Description:
  // Returns the full path of a file or dir in the cache if it's present
  const char* CachedFileFind ( const char * target, const char *dirname );

  const char* GetFilenameFromURI ( const char *uri );
  const char* AddCachePathToFilename ( const char *filename );
  const char* EncodeURI ( const char *uri );
    
  void CacheSizeCheck();
  void FreeBufferCheck();
  int GetFreeSpaceRemaining();
  int GetCachedFileList(const char *dirname);
  
  // Description:
  vtkGetMacro ( RemoteCacheLimit, int );
  vtkSetMacro ( RemoteCacheLimit, int );
  vtkGetMacro ( CurrentCacheSize, int );
  vtkSetMacro ( CurrentCacheSize, int );
  vtkGetMacro ( RemoteCacheFreeBufferSize, int );
  vtkSetMacro ( RemoteCacheFreeBufferSize, int );
  vtkGetMacro ( EnableForceRedownload, int );
  vtkSetMacro ( EnableForceRedownload, int );
  vtkGetMacro ( EnableRemoteCacheOverwriting, int );
  vtkSetMacro ( EnableRemoteCacheOverwriting, int );

  //BTX
    enum
    {
      NoCachedFile=0,
      OldCachedFile,
      CachedFile,
    };

  enum
    {
      InsufficientFreeBufferEvent =  21000,
      CacheLimitExceededEvent = 21001,
    };
  //ETX
  
 private:
  int RemoteCacheLimit;
  int CurrentCacheSize;
  int RemoteCacheFreeBufferSize;
  int EnableForceRedownload;
  int EnableRemoteCacheOverwriting;


  //BTX
  std::string RemoteCacheDirectory;
  std::vector< std::string > GetAllCachedFiles();
  // This array contains a list of cached file names (without paths)
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

