// MRML includes
#include "vtkURIHandler.h"
#include "vtkPermissionPrompter.h"

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro ( vtkURIHandler );
vtkCxxSetObjectMacro( vtkURIHandler, PermissionPrompter, vtkPermissionPrompter );
//----------------------------------------------------------------------------
vtkURIHandler::vtkURIHandler()
{
  this->LocalFile = nullptr;
  this->RequiresPermission = 0;
  this->PermissionPrompter = nullptr;
  this->Prefix = nullptr;
  this->Name = nullptr;
  this->HostName = nullptr;
}


//----------------------------------------------------------------------------
vtkURIHandler::~vtkURIHandler()
{
  this->LocalFile = nullptr;
  if ( this->PermissionPrompter != nullptr )
    {
    this->PermissionPrompter->Delete();
    this->PermissionPrompter = nullptr;
    }
  if ( this->Prefix != nullptr )
    {
    this->SetPrefix ( nullptr );
    }
  if ( this->Name != nullptr )
    {
    this->SetName ( nullptr );
    }
  if ( this->HostName != nullptr )
    {
    this->SetHostName (nullptr );
    }
}


//----------------------------------------------------------------------------
void vtkURIHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}


//----------------------------------------------------------------------------
void vtkURIHandler::SetLocalFile (FILE *localFile )
{
  this->LocalFile = localFile;
}


//----------------------------------------------------------------------------
int vtkURIHandler::ProgressCallback ( FILE * vtkNotUsed(outputFile), double dltotal,
                              double dlnow, double ultotal, double ulnow )
{
    if(ultotal == 0)
    {
    if(dltotal > 0)
      {
      std::cout << "<filter-progress>" << dlnow/dltotal
                << "</filter-progress>" << std::endl;
      }
    }
  else
    {
    std::cout << ulnow*100/ultotal << "%" << std::endl;
    }
  return 0;
}


//----------------------------------------------------------------------------
size_t vtkURIHandler::BufferedWrite ( char *buffer, size_t size, size_t nitems )
{
  if ( this->LocalFile != nullptr )
    {
    //this->LocalFile->write(buffer,size*nitems);
    size_t size_written = fwrite(buffer, sizeof(char), size*nitems, this->LocalFile);
    if (size_written != size*nitems)
      {
      std::cerr << "Error: BufferedWrite tried to write " << size*nitems << ", only wrote " << size_written << " items" << std::endl;
      return size_written;
      }
    else
      {
      size *= nitems;
      return size;
      }
    }
  else
    {
    return ( 0 );
    }
}

//----------------------------------------------------------------------------
void vtkURIHandler::StageFileRead ( const char * vtkNotUsed( source ), const char *  vtkNotUsed( destination ))
{
}

//----------------------------------------------------------------------------
void vtkURIHandler::StageFileWrite ( const char * vtkNotUsed( source ), const char *  vtkNotUsed( destination ))
{
}

//----------------------------------------------------------------------------
void vtkURIHandler::StageFileRead(const char * vtkNotUsed( source ),
                             const char * vtkNotUsed( destination ),
                             const char * vtkNotUsed( username ),
                             const char * vtkNotUsed( password ),
                             const char * vtkNotUsed( hostname ) )
{
}

//----------------------------------------------------------------------------
void vtkURIHandler::StageFileWrite(const char * vtkNotUsed( source ),
                              const char * vtkNotUsed( username ),
                              const char * vtkNotUsed( password ),
                              const char * vtkNotUsed( hostname ),
                              const char * vtkNotUsed( sessionID ) )
{
}

//----------------------------------------------------------------------------
void vtkURIHandler::InitTransfer ( )
{
}
