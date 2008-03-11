
#include "vtkCacheManager.h"
#include "vtkCallbackCommand.h"
#include "vtksys/Directory.hxx"

vtkStandardNewMacro ( vtkCacheManager );
vtkCxxRevisionMacro ( vtkCacheManager, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkCacheManager::vtkCacheManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->CachedFileList.clear();
  this->RemoteCacheLimit = 0;
  this->CurrentCacheSize = 0;
  this->RemoteCacheFreeBufferSize = 0;
  this->EnableForceRedownload = 0;
  // this->EnableRemoteCacheOverwriting = 1;

}


//----------------------------------------------------------------------------
vtkCacheManager::~vtkCacheManager()
{
   
  if (this->CallbackCommand)
    {
    this->CallbackCommand->Delete();
    }
  this->CachedFileList.clear();
  this->RemoteCacheLimit = 0;
  this->CurrentCacheSize = 0;
  this->RemoteCacheFreeBufferSize = 0;
  this->EnableForceRedownload = 0;
//  this->EnableRemoteCacheOverwriting = 1;

}


//----------------------------------------------------------------------------
void vtkCacheManager::SetRemoteCacheDirectory (const char *dir )
{
  std::string dirstring = dir ;
  int len = dirstring.size();

  if ( len > 0 )
    {
    std::string tst = dirstring.substr(len-1);
    //---
    //--- make sure to remove backslash on the end of the dirstring.
    //---
    if ( tst == "/" )
      {
      dirstring = dirstring.substr( 0, len-1 );
      //dirstring += "/";
      }
    this->RemoteCacheDirectory = dirstring;
    }
  else
    {
    vtkWarningMacro ( "Setting RemoteCacheDirectory to be a null string." );      
    this->RemoteCacheDirectory = "";
    }
}





//----------------------------------------------------------------------------
int vtkCacheManager::IsRemoteReference ( const char *uri )
{
  int index;
  std::string uriString (uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = uriString.find ( "://", 0 ) ) != std::string::npos )
    {
    prefix = uriString.substr ( 0, index );
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ( (index = prefix.find ( "]:", 0 ) ) != std::string::npos )
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
  if ( ( index = uriString.find ( "://", 0 ) ) != std::string::npos )
    {
    prefix = uriString.substr ( 0, index );
    //--- check to see if any leading bracketed characters are
    //--- in this part of the string.
    if ( (index = prefix.find ( "]:", 0 ) ) != std::string::npos )
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
  if ( ( index = uriString.find ( "://", 0 ) ) != std::string::npos )
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
int vtkCacheManager::GetCachedFileList ( const char *dirname )
{

//  vtksys_stl::string convdir = vtksys::SystemTools::ConvertToOutputPath ( dirname );
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
          vtksys_stl::string fullName = dirname;
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
            this->CachedFileList.push_back ( dir.GetFile(static_cast<unsigned long>(fileNum) ));
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
    if (uri == NULL)
    {
    return "(null)";
    }
  vtksys_stl::string kwInString = vtksys_stl::string(uri);
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
  char *returnString = NULL;
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
  vtksys_stl::string cachedir ( this->GetRemoteCacheDirectory() );
  if ( cachedir.c_str() != NULL )
    {
    vtksys_stl::string ret = cachedir;
    ret += "/";
    ret += filename;

    const char *outStr = ret.c_str();
    char *absoluteName = NULL;
    size_t n = strlen(outStr) + 1;
    char *cp1 = new char[n];
    const char *cp2 = (outStr);
    absoluteName = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    return  absoluteName ;

    }
  else
    {
    return ( NULL );
    }
}


//----------------------------------------------------------------------------
const char* vtkCacheManager::GetFilenameFromURI ( const char *uri )
{
  //--- this method should return the absolute path of the
  //--- file that a storage node will read from in cache
  //--- (the download destination file).


  if (uri == NULL)
    {
    vtkDebugMacro("GetFilenameFromURI: input uri is null");
    return "(null)";
    }
  vtksys_stl::string kwInString = vtksys_stl::string(uri);

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
  vtksys_stl::string filename = vtksys::SystemTools::GetFilenameName ( kwInString );

  vtkDebugMacro("GetFilenameFromURI: got filename name " << filename.c_str());

  //--- Create absolute path
  if (this->GetRemoteCacheDirectory() == NULL ||
      strcmp(this->GetRemoteCacheDirectory(), "") == 0)
    {
    vtkErrorMacro("GetFilenameFromURI: remote cache dir is not set! The file will appear in the current working dir.");
    }
  vtksys_stl::string absolute = this->GetRemoteCacheDirectory();
  absolute += "/";
  absolute += filename;
  filename = absolute;
  
  const char *inStr = filename.c_str();
  char *returnString = NULL;
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
  if (target == NULL)
    {
    return;
    }

  //--- discover if target already has Remote Cache Directory prepended to path.
  //--- if not, put it there.

  if (this->FindCachedFile( target, this->GetRemoteCacheDirectory() ) == NULL)
    {
    vtkErrorMacro("RemoveFromCache: can't find the target file " << target << ", so there's nothing to do, returning.");
    return;
    }
  std::string str = this->FindCachedFile( target, this->GetRemoteCacheDirectory() );
  if ( str.c_str() != NULL )
    {
    //--- remove the file or directory in str....
    vtkWarningMacro ( "Removing " << str.c_str() << " from disk and from record of cached files." );
    if ( vtksys::SystemTools::FileIsDirectory ( str.c_str() ) )
      {
      if ( !vtksys::SystemTools::RemoveADirectory ( str.c_str() ))
        {
        vtkWarningMacro ( "Unable to remove cached directory " << str.c_str() << "from disk." );
        }
      else
        {
        this->DeleteFromCachedFileList ( str.c_str() );
        this->UpdateCacheInformation ( );
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
        this->DeleteFromCachedFileList ( str.c_str() );
        this->UpdateCacheInformation ( );
        }
      }
    }
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
  if ( this->RemoteCacheDirectory.c_str() != NULL )
    {
    vtksys::SystemTools::RemoveADirectory ( this->RemoteCacheDirectory.c_str() );
    }
  if ( vtksys::SystemTools::MakeDirectory ( this->RemoteCacheDirectory.c_str() ) == false )
    {
    return 0;
    }
  this->UpdateCacheInformation();
  this->Modified();
  return 1;
}

  

//----------------------------------------------------------------------------
unsigned long vtkCacheManager::ComputeCacheSize( const char *dirName, unsigned long sz )
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
          cachesize += this->ComputeCacheSize (subdirString.c_str(), 0);
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
  this->CurrentCacheSize = static_cast<int>(cachesize);
  return (cachesize);
}

//----------------------------------------------------------------------------
void vtkCacheManager::CacheSizeCheck()
{
  
  //--- Compute size of the current cache
  this->ComputeCacheSize(this->RemoteCacheDirectory.c_str(), 0);
  //--- Invoke an event if cache size is exceeded.
  if ( this->CurrentCacheSize > this->RemoteCacheLimit )
    {
    // remove the file just downloaded?
     this->InvokeEvent ( vtkCacheManager::CacheLimitExceededEvent );
    }
}

//----------------------------------------------------------------------------
int vtkCacheManager::GetFreeCacheSpaceRemaining()
{

  int cachesize = this->ComputeCacheSize(this->RemoteCacheDirectory.c_str(), 0);
  int diff = ( this->RemoteCacheFreeBufferSize - cachesize );
  return ( diff );

}


//----------------------------------------------------------------------------
void vtkCacheManager::FreeCacheBufferCheck()
{

  int buf = this->GetFreeCacheSpaceRemaining();
  if ( buf < this->RemoteCacheFreeBufferSize )
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
  const char *result = NULL;
  char *returnString;
  size_t n;
  char *cp1;
  const char *cp2;
  
  if (target == NULL || dirname == NULL)
    {
    vtkErrorMacro("FindCachedFile: target or dirname null");
    return ( NULL );
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
        vtksys_stl::string fullName = dirname;
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
          if ( (result = this->FindCachedFile ( target, fullName.c_str() )) != NULL )
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
    return (NULL);
    }

  if ( result != NULL )
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
    return (NULL);
    }

}
