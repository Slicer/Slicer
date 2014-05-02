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
  this->LocalFile = NULL;
  this->RequiresPermission = 0;
  this->PermissionPrompter = NULL;
  this->Prefix = NULL;
  this->Name = NULL;
  this->HostName = NULL;
}


//----------------------------------------------------------------------------
vtkURIHandler::~vtkURIHandler()
{
  this->LocalFile = NULL;
  if ( this->PermissionPrompter != NULL )
    {
    this->PermissionPrompter->Delete();
    this->PermissionPrompter = NULL;
    }
  if ( this->Prefix != NULL )
    {
    this->SetPrefix ( NULL );
    }
  if ( this->Name != NULL )
    {
    this->SetName ( NULL );
    }
  if ( this->HostName != NULL )
    {
    this->SetHostName (NULL );
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
  if ( this->LocalFile != NULL )
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
