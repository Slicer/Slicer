
#include "vtkCacheManager.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"

#include <vtksys/Directory.hxx>
#include <vtksys/SystemTools.hxx>

#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro ( vtkCacheManager );

#define MB 1000000.0

//----------------------------------------------------------------------------
vtkCacheManager::vtkCacheManager()
{
  this->MRMLScene = nullptr;
  this->CallbackCommand = vtkCallbackCommand::New();
  this->CachedFileList.clear();
  //--- what seem reasonable default values here?
  this->RemoteCacheLimit = 200;
  this->RemoteCacheFreeBufferSize = 10;
  this->CurrentCacheSize = 0;
  this->EnableForceRedownload = 0;
  this->InsufficientFreeBufferNotificationFlag = 0;
  // this->EnableRemoteCacheOverwriting = 1;
  this->uriMap.clear();
}


//----------------------------------------------------------------------------
vtkCacheManager::~vtkCacheManager()
{

  this->MRMLScene = nullptr;
  this->uriMap.clear();
  if (this->CallbackCommand)
    {
    this->CallbackCommand->Delete();
    }
  this->CachedFileList.clear();
  this->RemoteCacheLimit = 0;
  this->CurrentCacheSize = 0;
  this->RemoteCacheFreeBufferSize = 0;
  this->EnableForceRedownload = 0;
  this->InsufficientFreeBufferNotificationFlag = 0;
//  this->EnableRemoteCacheOverwriting = 1;
}


//----------------------------------------------------------------------------
const char* vtkCacheManager::GetFileFromURIMap (const char *uri )
{
  std::string uriString (uri);

    //--- URI is first, local name is second
    std::map<std::string, std::string>::iterator iter = this->uriMap.find(uriString);
    if (iter != this->uriMap.end() )
      {
      return iter->second.c_str();
      }

  return nullptr;
}


//----------------------------------------------------------------------------
void vtkCacheManager::MapFileToURI ( const char *uri, const char *fname )
{
  if ( uri == nullptr || fname == nullptr )
    {
    vtkErrorMacro ( "MapFileToURI: got two null strings." );
    return;
    }

  std::string remote(uri);
  std::string local(fname);

  std::map <std::string, std::string>::iterator iter;
  //--- see if it's already here and update if so.

    //--- URI is first, local name is second
  int added = 0;
  for (iter = this->uriMap.begin();
       iter != this->uriMap.end();
       iter++)
    {
    if (iter->first == remote )
      {
      iter->second = local;
      added = 1;
      }
    }
  if ( !added )
    {
    this->uriMap.insert (std::make_pair (remote, local ));
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkCacheManager::SetRemoteCacheDirectory (const char *dir )
{
  std::string dirstring = dir ;
  size_t len = (int)dirstring.size();

  if ( len == 0 )
    {
    vtkWarningMacro ( "Setting RemoteCacheDirectory to be a null string." );
    }
  else
    {
    //---
    //--- make sure to remove backslash on the end of the dirstring.
    //---
    const char lastChar = dirstring.at(len-1);
    if ( lastChar == '/' || lastChar == '\\' )
      {
      dirstring = dirstring.substr( 0, len-1 );
      }
    }

  if (this->RemoteCacheDirectory == dirstring)
    {
    return;
    }

  this->RemoteCacheDirectory = dirstring;
  if (!vtksys::SystemTools::FileExists(this->RemoteCacheDirectory.c_str()))
    {
    vtksys::SystemTools::MakeDirectory(this->RemoteCacheDirectory.c_str());
    }
  // scan files in cache, it calls Modified
  this->UpdateCacheInformation();
}

//----------------------------------------------------------------------------
const char *vtkCacheManager::GetRemoteCacheDirectory ()
{
  return ( this->RemoteCacheDirectory.c_str() );
}

//----------------------------------------------------------------------------
int vtkCacheManager::IsRemoteReference ( const char *uri )
{
  int index;
  std::string uriString (uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = (int)(uriString.find ( "://", 0 )) ) != (int)(std::string::npos) )
    {
    prefix = uriString.substr ( 0, index );
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ( (index = (int)(prefix.find ( "]:", 0 )) ) != (int)(std::string::npos) )
      {
      //--- if so, strip off the bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr ( index+2);
      }
    if ( prefix == "file" )
      {
      return (0);
      }
    else
      {
      return (1);
      }
    }
  else
    {
    vtkDebugMacro ( "URI " << uri << " contains no file:// or other prefix." );
    //--- doesn't seem to be a :// in the string.
    return (0);
    }
}

//----------------------------------------------------------------------------
int vtkCacheManager::IsLocalReference ( const char *uri )
{
  int index;
  std::string uriString (uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = (int)(uriString.find ( "://", 0 )) ) != (int)(std::string::npos) )
    {
    prefix = uriString.substr ( 0, index );
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ( (index = (int)(prefix.find ( "]:", 0 )) ) != (int)(std::string::npos) )
      {
      //--- if so, strip off the bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr ( index+2);
      }
    if ( prefix == "file" )
      {
      return (1);
      }
    else
      {
      return (0);
      }
    }
  else
    {
    vtkWarningMacro ( "URI " << uri << " contains no file:// or other prefix." );
    return (0);
    }
}

//----------------------------------------------------------------------------
int vtkCacheManager::LocalFileExists ( const char *uri )
{
  int index;
  std::string uriString (uri);
  std::string prefix;
  std::string filename;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = (int)(uriString.find ( "://", 0 )) ) != (int)(std::string::npos) )
    {
    //--- is this the correct index???
    filename = uriString.substr ( index+3 );
    }
  else
    {
    filename = uri;
    }

  if ( vtksys::SystemTools::FileExists ( filename.c_str() ) )
    {
    return ( 1 );
    }
  else
    {
    return (0);
    }
}


//----------------------------------------------------------------------------
void vtkCacheManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "RemoteCacheDirectory: " << this->GetRemoteCacheDirectory() << "\n";
  os << indent << "RemoteCacheLimit: " << this->GetRemoteCacheLimit() << "\n";
  os << indent << "CurrentCacheSize: " << this->GetCurrentCacheSize() << "\n";
  os << indent << "RemoteCacheFreeBufferSize: " << this->GetRemoteCacheFreeBufferSize() << "\n";
  //os << indent << "EnableRemoteCacheOverwriting: " << this->GetEnableRemoteCacheOverwriting() << "\n";
  os << indent << "EnableForceRedownload: " << this->GetEnableForceRedownload() << "\n";
}


//----------------------------------------------------------------------------
std::vector< std::string > vtkCacheManager::GetAllCachedFiles ( )
{
  this->CachedFileList.clear();
  this->GetCachedFileList ( this->GetRemoteCacheDirectory() );
  return ( this->CachedFileList );
}


//----------------------------------------------------------------------------
std::vector< std::string > vtkCacheManager::GetCachedFiles ( ) const
{
  return this->CachedFileList;
}

//----------------------------------------------------------------------------
int vtkCacheManager::GetCachedFileList ( const char *dirname )
{

//  std::string convdir = vtksys::SystemTools::ConvertToOutputPath ( dirname );
  if ( vtksys::SystemTools::FileIsDirectory ( dirname ) )
    {
    vtksys::Directory dir;
    dir.Load( dirname );
    size_t fileNum;

    //--- get files in cache dir and add to vector of strings.
    for ( fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum )
      {
        {
        if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
            strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
          {
          std::string fullName = dirname;
          //--- add backslash to end if not present.
          if ( ( fullName.rfind ( "/", 0 )) != (fullName.size()-1) )
            {
            fullName += "/";
            }
          fullName += dir.GetFile(static_cast<unsigned long>(fileNum));

          //--- if the file is a directory, have to go inside and
          //--- do some recursive thing to add those files to cached list
          if(vtksys::SystemTools::FileIsDirectory(fullName.c_str()))
            {
            if ( ! this->GetCachedFileList ( fullName.c_str() ) )
              {
              return (0);
              }
            }
          else
            {
            this->CachedFileList.emplace_back(dir.GetFile(static_cast<unsigned long>(fileNum)));
            }
          }
        }
      }
    }
  else
    {
    vtkDebugMacro ( "vtkCacheManager::GetCachedFileList: Cache Directory "
                    << this->GetRemoteCacheDirectory() <<
                    " doesn't look like a directory.\n" );
    return (0);
    }
  return (1);
}



//----------------------------------------------------------------------------
const char* vtkCacheManager::EncodeURI ( const char *uri )
{
    if (uri == nullptr)
    {
    return "(null)";
    }
  std::string kwInString = std::string(uri);
  // encode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%", "%25");
  // encode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     " ", "%20");
  // encode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "'", "%27");
  // encode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     ">", "%3E");
  // encode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "<", "%3C");
  // encode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "\"", "%22");

  const char *inStr = kwInString.c_str();
  char *returnString = nullptr;
  size_t n = strlen(inStr) + 1;
  char *cp1 = new char[n];
  const char *cp2 = (inStr);
  returnString = cp1;
  do { *cp1++ = *cp2++; } while ( --n );
  return (returnString);
}



//----------------------------------------------------------------------------
const char* vtkCacheManager::AddCachePathToFilename ( const char *filename )
{
  std::string cachedir ( this->GetRemoteCacheDirectory() );
  if ( cachedir.c_str() != nullptr )
    {
    std::string ret = cachedir;
    ret += "/";
    ret += filename;

    const char *outStr = ret.c_str();
    char *absoluteName = nullptr;
    size_t n = strlen(outStr) + 1;
    char *cp1 = new char[n];
    const char *cp2 = (outStr);
    absoluteName = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    return  absoluteName ;

    }
  else
    {
    return ( nullptr );
    }
}


//----------------------------------------------------------------------------
const char* vtkCacheManager::GetFilenameFromURI ( const char *uri )
{
  //--- this method should return the absolute path of the
  //--- file that a storage node will read from in cache
  //--- (the download destination file).


  if (uri == nullptr)
    {
    vtkDebugMacro("GetFilenameFromURI: input uri is null");
    return "(null)";
    }

  const char *mapcheck = this->GetFileFromURIMap( uri );
  if ( mapcheck != nullptr )
    {
    return (mapcheck);
    }

  std::string kwInString = std::string(uri);

  std::string::size_type loc = kwInString.find ( "?" );
  if ( loc != kwInString.npos  )
    {
    kwInString = kwInString.substr (0, loc );
    vtkWarningMacro ( "Stripping question mark and trailing characters from uri." << kwInString.c_str() );
    }
  //--- First decode special characters
  // decode double quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%22", "\"");
  // decode less than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3C", "<");
  // decode greater than
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%3E", ">");
  // decode single quote
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%27", "'");
  // decode space
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%20", " ");
  // decode %
  vtksys::SystemTools::ReplaceString(kwInString,
                                     "%25", "%");

  //--- Now strip off all the uri prefix (does this work?)

  std::string newFileName;
  std::string fileName;
  std::string baseName;
  std::string extensionName;
  std::string tmpName = vtksys::SystemTools::GetFilenameName ( kwInString );
  std::string versionExtension;

  vtkDebugMacro("GetFilenameFromURI: got fileName name " << tmpName.c_str());

  //--- Now check for trailing version numbers.
//  std::string::size_type found = tmpName.find_first_of ( ".");
  std::string::size_type found = tmpName.find_last_of ( ".");
  if ( found != tmpName.npos )
    {
    //--- save the extension_version and baseName.
    versionExtension = tmpName.substr ( found );
    baseName = tmpName.substr ( 0, found );

    found = versionExtension.find_first_of ( "_");
    if ( found != versionExtension.npos )
      {
      //--- found a version number modifying the extension.
      extensionName = versionExtension.substr(0, found);
      }
    else
      {
      extensionName = versionExtension;
      }
    }
  if ( baseName.c_str() != nullptr )
    {
    newFileName = baseName.c_str();
    }
  if ( extensionName.c_str() != nullptr )
    {
    newFileName += extensionName.c_str();
    }

  //--- Create absolute path
  if (this->GetRemoteCacheDirectory() == nullptr ||
      strcmp(this->GetRemoteCacheDirectory(), "") == 0)
    {
    vtkErrorMacro("GetFilenameFromURI: remote cache dir is not set! The file will appear in the current working dir.");
    }
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath( this->GetRemoteCacheDirectory(), pathComponents);
  pathComponents.push_back ( newFileName );
  fileName = vtksys::SystemTools::JoinPath ( pathComponents );

  const char *inStr = fileName.c_str();
  char *returnString = nullptr;
  size_t n = strlen(inStr) + 1;
  char *cp1 = new char[n];
  const char *cp2 = (inStr);
  returnString = cp1;
  do { *cp1++ = *cp2++; } while ( --n );
  vtkDebugMacro("GetFilenameFromURI: returning " << returnString);

  return returnString;
}


//----------------------------------------------------------------------------
void vtkCacheManager::UpdateCacheInformation ( )
{
  //--- now recompute cache size
  // this->CurrentCacheSize = ?;

  //--- recompute free buffer size
  // this->RemoteCacheFreeBufferSize = ?;

  //--- and refresh list of cached files.
  this->CachedFileList.clear();
  this->GetCachedFileList ( this->GetRemoteCacheDirectory() );
  this->Modified();
}




//----------------------------------------------------------------------------
void vtkCacheManager::DeleteFromCachedFileList ( const char * target )
{

  std::string tstring = target;
  std::vector< std::string > tmp = this->CachedFileList;
  std::vector< std::string >::iterator it;
  this->CachedFileList.clear();

  for (it = tmp.begin(); it < tmp.end(); it++ )
      {
    // look at each filename in the list and keep all but filename to delete
    if ( *it != tstring )
      {
      this->CachedFileList.push_back ( *it );
      }
    }
  tmp.clear();

}





//----------------------------------------------------------------------------
void vtkCacheManager::DeleteFromCache( const char *target )
{
  if (target == nullptr)
    {
    return;
    }

  //--- discover if target already has Remote Cache Directory prepended to path.
  //--- if not, put it there.

  if (this->FindCachedFile( target, this->GetRemoteCacheDirectory() ) == nullptr)
    {
    vtkDebugMacro("RemoveFromCache: can't find the target file " << target << ", so there's nothing to do, returning.");
    return;
    }

  std::string str = this->FindCachedFile( target, this->GetRemoteCacheDirectory() );
  if ( str.c_str() != nullptr )
    {
    this->MarkNodesBeforeDeletingDataFromCache ( target );

    //--- remove the file or directory in str....
    vtkDebugMacro ( "Removing " << str.c_str() << " from disk and from record of cached files." );
    if ( vtksys::SystemTools::FileIsDirectory ( str.c_str() ) )
      {
      if ( !vtksys::SystemTools::RemoveADirectory ( str.c_str() ))
        {
        vtkWarningMacro ( "Unable to remove cached directory " << str.c_str() << "from disk." );
        }
      else
        {
        this->UpdateCacheInformation ( );
        this->InvokeEvent ( vtkCacheManager::CacheDeleteEvent );
        }
      }
    else
      {
      if ( !vtksys::SystemTools::RemoveFile ( str.c_str()  ))
        {
        vtkWarningMacro ( "Unable to remove cached file" << str.c_str() << "from disk." );
        }
      else
        {
        this->UpdateCacheInformation ( );
        this->InvokeEvent ( vtkCacheManager::CacheDeleteEvent );
        }
      }
    this->DeleteFromCachedFileList ( str.c_str() );
    }
}


//----------------------------------------------------------------------------
int vtkCacheManager::ClearCacheCheck()
{
  //
  // This method is called after ClearCache(),
  // to see if that method actually cleaned the cache.
  // If not, an event is invoked.
  // Things to do: get cache dir
  // check to see how many files are in it.
  // if more than... 0? invoke CacheDirtyEvent.
  //
  std::string cachedir = this->GetRemoteCacheDirectory();
  if ( cachedir.c_str() != nullptr )
    {
    unsigned long numFiles = vtksys::Directory::GetNumberOfFilesInDirectory( cachedir.c_str() );
    //--- assume method will return . and ..
    if ( numFiles > 2 )
      {
      this->InvokeEvent ( vtkCacheManager::CacheDirtyEvent );
      return 0;
      }
    }
  return 1;
}



//----------------------------------------------------------------------------
int vtkCacheManager::ClearCache()
{

  //--- Careful! Before making this call, prompt user
  //--- with the RemoteCacheDirectory name and
  //--- ask for confirmation whether to delete the
  //--- directory and all of its contents...
  //--- Removes the CacheDirectory all together
  //--- and then creates the directory again.
  if ( this->RemoteCacheDirectory.c_str() != nullptr )
    {
    this->MarkNodesBeforeDeletingDataFromCache ( this->RemoteCacheDirectory.c_str() );
    vtksys::SystemTools::RemoveADirectory ( this->RemoteCacheDirectory.c_str() );
    }
  if ( vtksys::SystemTools::MakeDirectory ( this->RemoteCacheDirectory.c_str() ) == false )
    {
    vtkWarningMacro ( "Cache cleared: Error: unable to recreate cache directory after deleting its contents." );
    return 0;
    }
  this->UpdateCacheInformation();
  this->InvokeEvent ( vtkCacheManager::CacheClearEvent );
  return 1;
}



//----------------------------------------------------------------------------
float vtkCacheManager::GetCurrentCacheSize ()
{
  if ( this->RemoteCacheDirectory.c_str() == nullptr )
    {
    return (0.0);
    }
  float size = this->ComputeCacheSize ( this->RemoteCacheDirectory.c_str(), 0 );
  this->SetCurrentCacheSize ( size );
  return ( this->CurrentCacheSize );

}


//----------------------------------------------------------------------------
void vtkCacheManager::MarkNode ( std::string str )
{
  //--- Find the MRML node that points to this file in cache.
  //--- If such a node exists, mark it as modified since read,
  //--- so that a user will be prompted to save the
  //--- data elsewhere (since it'll be deleted from cache.)
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLStorableNode" );
  vtkMRMLStorableNode *node;
  std::string uri;
  for ( int n=0; n < nnodes; n++ )
    {
    node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNthNodeByClass (n, "vtkMRMLStorableNode" ));
    if ( node != nullptr )
      {
      int numStorageNodes = node->GetNumberOfStorageNodes();
      for (int i = 0; i < numStorageNodes; i++)
        {
        if (node->GetNthStorageNode(i) != nullptr )
          {
          uri = node->GetNthStorageNode(i)->GetFullNameFromFileName();
          if ( str == uri )
            {
            node->GetNthStorageNode(i)->InvalidateFile();
            }
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkCacheManager::MarkNodesBeforeDeletingDataFromCache (const char *target )
{
  //--- Target is a file or directory marked for deletion in cache.
  //--- If target is a file, this method finds any node holding a
  //--- reference to it and marks that node as ModifiedSinceRead.
  //--- If target is a directory, the method traverses the directory
  //--- and any subdirectories, and marks nodes holding references
  //--- to any of the files within as ModifiedSinceRead.
  //--- might be a slow performer....(?)


  if ( target != nullptr )
    {
    std::string testFile;
    std::string longName;;
    std::string subdirString;
    if ( vtksys::SystemTools::FileIsDirectory ( target ) )
      {
      vtkDebugMacro("MarkNodesBeforeDeletingDataFromCache: target is a directory: " << target);
      vtksys::Directory dir;
      dir.Load( target );
      size_t fileNum;
      //--- get files in cache dir and add to vector of strings.
      for ( fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum )
        {
        if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
            strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
          {
          //--- test to see if the file is a directory;
          //--- if so, go inside and count up file sizes, return value
          subdirString = target;
          subdirString += "/";
          subdirString += dir.GetFile  (static_cast<unsigned long>(fileNum));
          if (vtksys::SystemTools::FileIsDirectory(subdirString.c_str()))
            {
            ///--- check in subdir too...
            this->MarkNodesBeforeDeletingDataFromCache( subdirString.c_str());
            }
          else
            {
            //--- mark any nodes that point to this file as modified since read.
            longName = target;
            longName += "/";
            testFile = dir.GetFile (static_cast<unsigned long>(fileNum));
            longName += testFile;
            this->MarkNode ( longName );
            }
          }
        }
      }
    else
      {
      if ( vtksys::SystemTools::FileExists ( target ))
        {
        this->MarkNode ( target );
        }
      }
    }
}



//----------------------------------------------------------------------------
float vtkCacheManager::ComputeCacheSize( const char *dirName, unsigned long sz )
{

  //--- Traverses cache directory and computes the combined size
  //--- of all files. I guess this is a reasonable guess to the cache size,
  //--- subdirectory size notwithstanding.
  //--- TODO: is there a more accurate way to assess?

  unsigned long cachesize = sz;
  std::string testFile;
  std::string longName;;
  std::string subdirString;
  if ( vtksys::SystemTools::FileIsDirectory ( dirName ) )
    {
    vtkDebugMacro("FindCachedFile: dirName is a directory: " << dirName);
    vtksys::Directory dir;
    dir.Load( dirName );
    size_t fileNum;
    cachesize += vtksys::SystemTools::FileLength (dirName );

    //--- get files in cache dir and add to vector of strings.
    for ( fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum )
      {
      if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
          strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
        {
        //--- test to see if the file is a directory;
        //--- if so, go inside and count up file sizes, return value
        subdirString = dirName;
        subdirString += "/";
        subdirString += dir.GetFile  (static_cast<unsigned long>(fileNum));
        if (vtksys::SystemTools::FileIsDirectory(subdirString.c_str()))
          {
          ///---compute dir and filename for recursive hunt
          cachesize += (long unsigned int)ceil(this->ComputeCacheSize (subdirString.c_str(), 0));
          }
        else
          {
          longName = dirName;
          longName += "/";
          testFile = dir.GetFile (static_cast<unsigned long>(fileNum));
          longName += testFile;
          //--- get its size
          cachesize += vtksys::SystemTools::FileLength (longName.c_str() );
          }
        }
      }
    }
  else
    {
    vtkDebugMacro ( "vtkCacheManager::ComputeCacheSize: Cache Directory "
                    << this->GetRemoteCacheDirectory() <<
                    " doesn't look like a directory. \n" );
    return (-1);
    }

  int byteSize = static_cast<int>(cachesize );
  this->CurrentCacheSize =  (float)byteSize / MB;
  return (this->CurrentCacheSize);
}

//----------------------------------------------------------------------------
void vtkCacheManager::CacheSizeCheck()
{

  //--- Compute size of the current cache
  this->ComputeCacheSize(this->RemoteCacheDirectory.c_str(), 0);
  //--- Invoke an event if cache size is exceeded.
  if ( this->CurrentCacheSize > (float) (this->RemoteCacheLimit) )
    {
    // remove the file just downloaded?
     this->InvokeEvent ( vtkCacheManager::CacheLimitExceededEvent );
    }
}

//----------------------------------------------------------------------------
float vtkCacheManager::GetFreeCacheSpaceRemaining()
{

  float cachesize = this->ComputeCacheSize(this->RemoteCacheDirectory.c_str(), 0);
  // cache limit - current cache size = total space left in cache.
  // total space in cache - free buffer size = amount that can be used.
  float diff = ( float (this->RemoteCacheLimit) - cachesize );
  diff = diff - (float)this->RemoteCacheFreeBufferSize;
  return ( diff  );

}


//----------------------------------------------------------------------------
void vtkCacheManager::FreeCacheBufferCheck()
{

  float buf = this->GetFreeCacheSpaceRemaining();
  if ( (buf*MB) < (float) (this->RemoteCacheFreeBufferSize)*MB )
    {
    this->InvokeEvent ( vtkCacheManager::InsufficientFreeBufferEvent );
    }
}



//----------------------------------------------------------------------------
int vtkCacheManager::CachedFileExists ( const char *filename )
{
  if ( vtksys::SystemTools::FileExists ( filename ) )
    {
    return 1;
    }
  else
    {
    //--- check to see if RemoteCacheDirectory/filename exists.
    std::string testFile = this->RemoteCacheDirectory;
    testFile += "/";
    testFile += filename;
    if ( vtksys::SystemTools::FileExists ( testFile.c_str() ))
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
}

//----------------------------------------------------------------------------
const char* vtkCacheManager::FindCachedFile ( const char * target, const char *dirname )
{
  std::string testFile;
  const char *result = nullptr;
  char *returnString;
  size_t n;
  char *cp1;
  const char *cp2;

  if (target == nullptr || dirname == nullptr)
    {
    vtkErrorMacro("FindCachedFile: target or dirname null");
    return ( nullptr );
    }

  if ( vtksys::SystemTools::FileIsDirectory ( dirname ) )
    {
    vtkDebugMacro("FindCachedFile: dirname is a directory: " << dirname);
    vtksys::Directory dir;
    dir.Load( dirname );
    size_t fileNum;

    //--- get files in cache dir and add to vector of strings.
    for ( fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum )
      {
      if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
          strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
        {
        testFile = dir.GetFile (static_cast<unsigned long>(fileNum));
        //--- Check for match to target
        //--- does the file or directory match the target?
        if ( !strcmp ( target, testFile.c_str() ))
          {
          //--- append the directory
          std::string accum = dirname;
          accum += "/";
          testFile = accum + testFile;
          result = testFile.c_str();
          n = strlen(result) + 1;
          cp1 = new char [n];
          cp2 = (result);
          returnString = cp1;
          do { *cp1++ = *cp2++; } while ( --n );
          return returnString;
          }
        //--- does the file or directory match the target with full path?
        //--- add backslash if missing
        std::string fullName = dirname;
        if ( ( fullName.rfind ( "/", 0 )) != (fullName.size()-1) )
          {
          fullName += "/";
          }
        fullName += testFile;
        if ( !strcmp ( target, fullName.c_str() ))
          {
          result = fullName.c_str();
          n = strlen(result) + 1;
          cp1 = new char[n];
          cp2 = (result);
          returnString = cp1;
          do { *cp1++ = *cp2++; } while ( --n );
          return returnString;
          }

        //--- if no match, and the file is a directory, go inside and
        //--- do some recursive thing to add those files to cached list
        if(vtksys::SystemTools::FileIsDirectory(fullName.c_str()))
          {
          ///---compute dir and filename for recursive hunt
          if ( (result = this->FindCachedFile ( target, fullName.c_str() )) != nullptr )
            {
            n = strlen(result) + 1;
            cp1 = new char[n];
            cp2 = (result);
            returnString = cp1;
            do { *cp1++ = *cp2++; } while ( --n );
            return returnString;
            }
          }
        }
      }
    }
  else
    {
    vtkDebugMacro ( "vtkCacheManager::GetCachedFileList: Cache Directory "
                    << this->GetRemoteCacheDirectory() <<
                    " doesn't look like a directory. \n" );
    return (nullptr);
    }

  if ( result != nullptr )
    {
    n = strlen(result) + 1;
    cp1 = new char[n];
    cp2 = (result);
    returnString = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    return returnString;
    }
  else
    {
    return (nullptr);
    }

}
