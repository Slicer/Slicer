#include "vtkObjectFactory.h"
#include "vtkFetchMIWriter.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIWriter );
vtkCxxRevisionMacro ( vtkFetchMIWriter, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIWriter::vtkFetchMIWriter ( )
{
  this->DocumentDeclarationFilename = NULL;
  this->HeaderFilename = NULL;
  this->MetadataFilename = NULL;
  this->Name = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMIWriter::~vtkFetchMIWriter ( )
{
  if ( this->DocumentDeclarationFilename )
    {
    delete [] this->DocumentDeclarationFilename;
    this->DocumentDeclarationFilename = NULL;
    }
  if ( this->HeaderFilename )
    {
    delete [] this->HeaderFilename;
  this->HeaderFilename= NULL;
    }
  if ( this->MetadataFilename )
    {
    delete [] this->MetadataFilename;
    this->MetadataFilename = NULL;
    }
  if ( this->Name )
    {
    this->Name = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIWriter::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    if ( this->DocumentDeclarationFilename )
      {
      os << indent << "DocumentDeclarationFilename: " << this->GetDocumentDeclarationFilename ( ) << "\n";
      }
    else
      {
      os << indent << "DocumentDeclarationFilename: NULL \n";
      }
    if ( this->HeaderFilename )
      {
      os << indent << "HeaderFilename: " << this->GetHeaderFilename ( ) << "\n";
      }
    else
      {
      os << indent << "HeaderFilename: NULL \n";
      }
    if ( this->MetadataFilename )
      {
      os << indent << "MetadataFilename: " << this->GetMetadataFilename ( ) << "\n";
      }
    else
      {
      os << indent << "MetadataFilename: NULL \n";
      }
    if ( this->Name )
      {
      os << indent << "Name: " << this->GetName() << "\n";
      }
    else
      {
      os << indent << "Name: NULL \n";
      }
    
}










