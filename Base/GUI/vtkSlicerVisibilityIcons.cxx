
#include "vtkObjectFactory.h"
#include "vtkSlicerVisibilityIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerVisibilityIcons );
vtkCxxRevisionMacro ( vtkSlicerVisibilityIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerVisibilityIcons::vtkSlicerVisibilityIcons ( )
{
    this->VisibleIcon = vtkKWIcon::New ( );
    this->InvisibleIcon = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerVisibilityIcons::~vtkSlicerVisibilityIcons ( )
{

    if ( this->VisibleIcon ) {
            this->VisibleIcon->Delete ( );
            this->VisibleIcon = NULL;
        }
    if ( this->InvisibleIcon ) {
            this->InvisibleIcon->Delete ( );
            this->InvisibleIcon = NULL;
        }

}

//---------------------------------------------------------------------------
void vtkSlicerVisibilityIcons::AssignImageDataToIcons ( )
{
    // SliceVisibleIcon
    this->VisibleIcon->SetImage( image_SliceVisible,
                                image_SliceVisible_width,
                                image_SliceVisible_height,
                                image_SliceVisible_pixel_size, 0, 0);

    this->InvisibleIcon->SetImage( image_SliceInvisible,
                                image_SliceInvisible_width,
                                image_SliceInvisible_height,
                                image_SliceInvisible_pixel_size, 0, 0);

}



//---------------------------------------------------------------------------
void vtkSlicerVisibilityIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerVisibilityIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "VisibleIcon: " << this->GetVisibleIcon ( ) << "\n";
    os << indent << "InvisibleIcon: " << this->GetInvisibleIcon ( ) << "\n";

}
