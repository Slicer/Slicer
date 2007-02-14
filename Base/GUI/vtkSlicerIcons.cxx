

#include "vtkObjectFactory.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerIcons );

//---------------------------------------------------------------------------
vtkSlicerIcons::vtkSlicerIcons ( )
{
}


//---------------------------------------------------------------------------
vtkSlicerIcons::~vtkSlicerIcons ( )
{
}


//---------------------------------------------------------------------------
void vtkSlicerIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerIcons: " << this->GetClassName ( ) << "\n";

}
