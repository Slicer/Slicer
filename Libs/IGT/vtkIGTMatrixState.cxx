
#include "vtkIGTMatrixState.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"

// Constructors/Destructors
//  
vtkStandardNewMacro (vtkIGTMatrixState);
vtkCxxRevisionMacro (vtkIGTMatrixState, "$Revision: 1.0 $");


vtkIGTMatrixState::vtkIGTMatrixState ( ) {
  this->Matrix = vtkMatrix4x4::New();

}

vtkIGTMatrixState::~vtkIGTMatrixState ( ) { 
}


//  
// Methods

void vtkIGTMatrixState::Init() {

}

void vtkIGTMatrixState::setTimestamp ( int new_var ) {
  m_timestamp = new_var;
}


int vtkIGTMatrixState::getTimestamp ( ) {
  return m_timestamp;
}

void vtkIGTMatrixState::PrintSelf(ostream& os, vtkIndent indent)
{

}


