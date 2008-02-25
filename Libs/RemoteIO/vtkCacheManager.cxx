
#include "vtkCacheManager.h"
#include "vtkCallbackCommand.h"


vtkStandardNewMacro ( vtkCacheManager );
vtkCxxRevisionMacro ( vtkCacheManager, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkCacheManager::vtkCacheManager()
{
  this->CallbackCommand = vtkCallbackCommand::New();
  this->CachedFileList.clear();
  this->CacheDirectoryName.clear();
  this->MaximumCacheSize = 0;
  this->CurrentCacheSize = 0;
  this->FreeBufferSize = 0;
  
}


//----------------------------------------------------------------------------
vtkCacheManager::~vtkCacheManager()
{
   
  if (this->CallbackCommand)
    {
    this->CallbackCommand->Delete();
    }
  this->CachedFileList.clear();
  this->CacheDirectoryName.clear();
  this->MaximumCacheSize = 0;
  this->CurrentCacheSize = 0;
  this->FreeBufferSize = 0;

}


//----------------------------------------------------------------------------
void vtkCacheManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "CacheDirectoryName: " << this->GetCacheDirectoryName() << "\n";
  os << indent << "MaximumCacheSize: " << this->GetMaximumCacheSize() << "\n";
  os << indent << "CurrentCacheSize: " << this->GetCurrentCacheSize() << "\n";
  os << indent << "FreeBufferSize: " << this->GetFreeBufferSize() << "\n";
}


//----------------------------------------------------------------------------
std::vector< std::string > vtkCacheManager::GetCachedFileList ( )
{
  return ( this->CachedFileList );
}


//----------------------------------------------------------------------------
void vtkCacheManager::AddFileToCachedFileList ( const char *filename )
{
  this->CachedFileList.push_back ( filename );
}

//----------------------------------------------------------------------------
void vtkCacheManager::RemoveFileFromCachedFileList ( const char * filename )
{
  
  const char *str;
  std::vector< std::string > tmp = this->CachedFileList;
  this->CachedFileList.clear();
  
  for ( unsigned int i=0; i < tmp.size(); i++ )
    {
    // look at eachh filename in the list
    str = tmp[i].c_str();
    // if it matches the target, remove it.
    if (  (strcmp ( str, filename ) ))
      {
      this->CachedFileList.push_back ( str );
      }
    }
  tmp.clear();
}


//----------------------------------------------------------------------------
void vtkCacheManager::RemoveFileFromCache( const char *filename )
{
  std::string str;

  
  // append the cache directory onto the name of the file
  str = this->CacheDirectoryName + filename;
  // clear the filename from the file list.
  vtkWarningMacro ( "Removing cache file" << str.c_str() << "from disk and from record of cached files." );
  this->RemoveFileFromCachedFileList ( str.c_str() );
  // and remove the actual file.
  // ?
}

//----------------------------------------------------------------------------
void vtkCacheManager::ClearCache()
{

  for (unsigned int i=0; i < this->CachedFileList.size(); i++ )
    {
    this->RemoveFileFromCache ( this->CachedFileList[i].c_str() );
    }
  this->Modified();
}

  

//----------------------------------------------------------------------------
void vtkCacheManager::CacheSizeCheck()
{
  
  // todo: this warns if you are already over.
  if ( this->CurrentCacheSize > this->MaximumCacheSize )
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
  return ( this->FreeBufferSize );

}


//----------------------------------------------------------------------------
void vtkCacheManager::FreeBufferCheck()
{
  int buf = this->GetFreeSpaceRemaining();
  if ( buf < this->FreeBufferSize )
    {
    this->InvokeEvent ( vtkCacheManager::InsufficientFreeBufferEvent );
    }
}


//----------------------------------------------------------------------------
int vtkCacheManager::CachedFileCheck( const char * filename )
{

  int current_session = 0;
  int on_disk = 0;
  for (unsigned int i=0; i < this->CachedFileList.size(); i++ )
    {
    // can we find name in list?
    if (!( strcmp ( filename, this->CachedFileList[i].c_str() )))
      {
      current_session = 1;
      }
    }
  // confirm, can we find name on disk?
  
  if ( current_session  && on_disk)
    {
    // load file
    return (vtkCacheManager::CachedFile);
    }
  else if ( !current_session && on_disk )
    {
    return (vtkCacheManager::OldCachedFile);
    }
  else
    {
    return (vtkCacheManager::NoCachedFile);
    }

}
