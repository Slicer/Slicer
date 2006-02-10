
#include "vtkObjectFactory.h"
#include "vtkSlicerApplicationLogic.h"

vtkStandardNewMacro (vtkSlicerApplicationLogic );
vtkCxxRevisionMacro (vtkSlicerApplicationLogic, "$Revision: 1.0 $" );

vtkSlicerApplicationLogic::vtkSlicerApplicationLogic  ( ) {
    this->state = 11.0;
}

vtkSlicerApplicationLogic::~vtkSlicerApplicationLogic ( ) {
}

void vtkSlicerApplicationLogic::SetMyState ( double s ) {
    this->state = s;
}

double vtkSlicerApplicationLogic::GetMyState ( ) {
    return this->state;
}

