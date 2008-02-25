#ifndef __vtkCacheManager_h
#define __vtkCacheManager_h

#include <vtkRemoteIOConfigure.h>
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkRemoteIO.h"
#include "vtkUnsignedLongArray.h"
#include "vtkIntArray.h"

class vtkCallbackCommand;

#ifndef vtkObjectPointer
#define vtkObjectPointer(xx) (reinterpret_cast <vtkObject **>( (xx) ))
#endif


class VTK_RemoteIO_EXPORT vtkCacheManager : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkCacheManager *New();
  vtkTypeRevisionMacro(vtkCacheManager, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Sets the name of the directory to use for local file caching
  void SetCacheDirectoryName (const char *dir ) {
  this->CacheDirectoryName = vtksys_stl::string(dir);
  };
  // Description:
  // Returns the name of the directory to use for local file caching
  const char *GetCacheDirectoryName () {
  return this->CacheDirectoryName.c_str();
  };

  // Description:
  // Adds a new filename to the list of locally cached files.
  void AddFileToCachedFileList ( const char *filename );
  // Description:
  // Removes a filename from the list of locally cached files.
  void RemoveFileFromCachedFileList ( const char * filename );
  // Description:
  // Removes a file from the cachedir and
  //removes its filename from CachedFileList
  void RemoveFileFromCache ( const char *filename );
  // Description:
  // Removes all files from the cachedir
  // and removes all filenames from CachedFileList
  void ClearCache ( );
  
  // Description:
  // Returns 1 if filename is in cache directory and is readable, 0 if not.
  int CachedFileCheck ( const char * filename);
    
  void CacheSizeCheck();
  void FreeBufferCheck();
  int GetFreeSpaceRemaining();
  
  // Description:
  vtkGetMacro ( MaximumCacheSize, int );
  vtkSetMacro ( MaximumCacheSize, int );
  vtkGetMacro ( CurrentCacheSize, int );
  vtkSetMacro ( CurrentCacheSize, int );
  vtkGetMacro ( FreeBufferSize, int );
  vtkSetMacro ( FreeBufferSize, int );
  
 private:
  int MaximumCacheSize;
  int CurrentCacheSize;
  int FreeBufferSize;

  //BTX
  std::string CacheDirectoryName;
  std::vector< std::string > GetCachedFileList();
  std::vector< std::string > CachedFileList;

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

