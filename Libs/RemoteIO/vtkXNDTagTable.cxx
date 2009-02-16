#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkXNDTagTable.h"
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkXNDTagTable );
vtkCxxRevisionMacro(vtkXNDTagTable, "$Revision: 1.9.12.1 $");


//----------------------------------------------------------------------------
vtkXNDTagTable::vtkXNDTagTable()
{
}


//----------------------------------------------------------------------------
vtkXNDTagTable::~vtkXNDTagTable()
{
}


//----------------------------------------------------------------------------
void vtkXNDTagTable::PrintSelf ( ostream &os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "XNDTagTable: " << this->GetClassName ( ) << "\n";
}



//----------------------------------------------------------------------------
void vtkXNDTagTable::Initialize( )
{

  this->ClearTagTable();

  // Default Tags for XNAT Desktop
  this->AddUniqueTag ( "Experiment", "none" );
  this->AddUniqueTag ( "Project", "none" );
  this->AddUniqueTag ( "Scan", "none");
  this->AddUniqueTag ( "Subject", "none");  
  this->AddUniqueTag ( "Modality", "none");

  // Tags required for Slicer
  this->AddUniqueTag ( "SlicerDataType", "MRML" );
  this->SelectTag ( "SlicerDataType");
  
}


