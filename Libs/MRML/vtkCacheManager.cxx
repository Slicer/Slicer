
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
int vtkCacheManager::IsRemoteReference ( const char *uri )
{
  int index;
  std::string uriString (uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = uriString.find ( "://", 0 ) ) != std::string::npos )
    {
    prefix = uriString.substr ( 0, index-1 );
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
    vtkWarningMacro ( "URI " << uri << " contains no file:// or other prefix." );      
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
    prefix = uriString.substr ( 0, index-1 );
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
    vtksys_stl::string comp ( filename );
    vtksys_stl::string ret = cachedir + comp;
    return ( ret.c_str() );
    }
  else
    {
    return ( NULL );
    }
}


//----------------------------------------------------------------------------
const char* vtkCacheManager::GetFilenameFromURI ( const char *uri )
{
  if (uri == NULL)
    {
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

  //--- Now strip off the prefix (does this work?)
  vtksys_stl::string filename = vtksys::SystemTools::GetFilenameName ( kwInString );

  
   const char *inStr = filename.c_str();
  char *returnString = NULL;
  size_t n = strlen(inStr) + 1;
  char *cp1 = new char[n];
  const char *cp2 = (inStr);
  returnString = cp1;
  do { *cp1++ = *cp2++; } while ( --n );

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
void vtkCacheManager::RemoveFromCachedFileList ( const char * target )
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
void vtkCacheManager::RemoveFromCache( const char *target )
{

  std::string str = this->CachedFileFind( target, this->GetRemoteCacheDirectory() );

  if ( str.c_str() != NULL )
    {
    //--- remove the file or directory in str....
    vtkWarningMacro ( "Removing " << str.c_str() << "from disk and from record of cached files." );
    if ( vtksys::SystemTools::FileIsDirectory ( str.c_str() ) )
      {
      if ( !vtksys::SystemTools::RemoveADirectory ( str.c_str() ))
        {
        vtkWarningMacro ( "Unable to remove cached directory " << str.c_str() << "from disk." );
        }
      else
        {
        this->RemoveFromCachedFileList ( str.c_str() );
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
        this->RemoveFromCachedFileList ( str.c_str() );
        this->UpdateCacheInformation ( );
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkCacheManager::ClearCache()
{

  for (unsigned int i=0; i < this->CachedFileList.size(); i++ )
    {
    this->RemoveFromCache ( this->CachedFileList[i].c_str() );
    }
  this->UpdateCacheInformation();
  this->Modified();
}

  

//----------------------------------------------------------------------------
void vtkCacheManager::CacheSizeCheck()
{
  
  // todo: this warns if you are already over.
  if ( this->CurrentCacheSize > this->RemoteCacheLimit )
    {
    // remove the file just downloaded?
     this->InvokeEvent ( vtkCacheManager::CacheLimitExceededEvent );
    }
}

//----------------------------------------------------------------------------
int vtkCacheManager::GetFreeSpaceRemaining()
{
  //TODO: figure out how to compute
  // for testing...
  return ( this->RemoteCacheFreeBufferSize );

}


//----------------------------------------------------------------------------
void vtkCacheManager::FreeBufferCheck()
{
  int buf = this->GetFreeSpaceRemaining();
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
    return 0;
    }
}

//----------------------------------------------------------------------------
const char* vtkCacheManager::CachedFileFind ( const char * target, const char *dirname )
{

  std::string testFile;
  const char *result = NULL;
    
  if ( vtksys::SystemTools::FileIsDirectory ( dirname ) )
    {
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
        vtksys_stl::string fullName = dirname;
        //--- add backslash if missing
        if ( ( fullName.rfind ( "/", 0 )) != (fullName.size()-1) )
          {
          fullName += "/";
          }
        fullName += testFile;
          
        //--- Check for match to target
        //--- does the file or directory match the target?
        if ( !strcmp ( target, testFile.c_str() ))
          {
          return ( fullName.c_str() );
          }
        //--- does the file or directory match the target with full path?
        if ( !strcmp ( target, fullName.c_str() ))
          {
          return ( fullName.c_str() );
          }

        //--- if no match, and the file is a directory, go inside and
        //--- do some recursive thing to add those files to cached list
        if(vtksys::SystemTools::FileIsDirectory(fullName.c_str()))
          {
          ///---compute dir and filename for recursive hunt
          if ( (result = this->CachedFileFind ( target, fullName.c_str() )) != NULL )
            {
            return ( result );
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
  return ( result );
}
