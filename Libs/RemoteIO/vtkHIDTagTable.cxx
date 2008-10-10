#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkHIDTagTable.h"
#include "vtkStringArray.h"
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkHIDTagTable );
vtkCxxRevisionMacro(vtkHIDTagTable, "$Revision: 1.0 $");


//----------------------------------------------------------------------------
vtkHIDTagTable::vtkHIDTagTable()
{
}


//----------------------------------------------------------------------------
vtkHIDTagTable::~vtkHIDTagTable()
{
}



//----------------------------------------------------------------------------
void vtkHIDTagTable::PrintSelf ( ostream &os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "HIDTagTable: " << this->GetClassName ( ) << "\n";
}


//----------------------------------------------------------------------------
void vtkHIDTagTable::Initialize( )
{
  //--- default tags for HID right now are
  //--- subject
  //--- experiment
  //--- file_type
  this->ClearTagTable();
  this->AddOrUpdateTag ( "Subject", "NULL" );
  this->AddOrUpdateTag ( "Experiment", "NULL" );
  this->AddOrUpdateTag ( "File_Type", "NULL" );
  this->SelectTag ( "File_Type");

}




