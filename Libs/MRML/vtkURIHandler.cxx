
#include "vtkObjectFactory.h"
#include "vtkURIHandler.h"


vtkCxxRevisionMacro ( vtkURIHandler, "$Revision: 1.0 $" );
//----------------------------------------------------------------------------
vtkURIHandler::vtkURIHandler()
{
  this->LocalFile = NULL;
  this->RequiresPermission = 0;
  this->PermissionPrompter = NULL;
  this->Prefix = NULL;
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
int vtkURIHandler::ProgressCallback ( FILE *outputFile, double dltotal,
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
    fwrite(buffer, sizeof(char), size*nitems, this->LocalFile);
    size *= nitems;
    return size;
    }
  else
    {
    return ( 0 );
    }
}

