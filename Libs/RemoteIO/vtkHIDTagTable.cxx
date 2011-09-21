#include "vtkObjectFactory.h"
#include "vtkHIDTagTable.h"

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
  this->AddOrUpdateTag ( "Subject", "000670986943" );
  this->AddOrUpdateTag ( "Experiment", "fBIRNPhaseII__0010" );
  this->AddOrUpdateTag ( "File_Type", "MRML" );
  this->SelectTag ( "File_Type");
  this->SelectTag ( "Subject");
  this->SelectTag ( "Experiment");
}
