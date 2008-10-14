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
  this->AddOrUpdateTag ( "SlicerDataType", "MRML" );
}


