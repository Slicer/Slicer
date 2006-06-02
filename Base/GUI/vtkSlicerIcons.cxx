

#include "vtkObjectFactory.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerIcons );
vtkCxxRevisionMacro ( vtkSlicerIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerIcons::vtkSlicerIcons ( )
{
    SlicerImagePath = "C:/cygwin/home/wjp/slicer3/Base/GUI/Images/";
}


//---------------------------------------------------------------------------
vtkSlicerIcons::~vtkSlicerIcons ( )
{
}



//---------------------------------------------------------------------------
void vtkSlicerIcons::DeleteIcon ( vtkKWIcon *icon )
{
    if ( icon != NULL ) {
        icon->Delete ( );
        icon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "SlicerImagePath: " << this->GetSlicerImagePath ( ) << "\n";

}
