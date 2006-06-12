

#include "vtkObjectFactory.h"
#include "vtkSlicerLogoIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerLogoIcons );
vtkCxxRevisionMacro ( vtkSlicerLogoIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerLogoIcons::vtkSlicerLogoIcons ( )
{
    this->SlicerLogo = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerLogoIcons::~vtkSlicerLogoIcons ( )
{

    if ( this->SlicerLogo ) {
            this->SlicerLogo->Delete ( );
            this->SlicerLogo = NULL;
        }
}

//---------------------------------------------------------------------------
void vtkSlicerLogoIcons::AssignImageDataToIcons ( )
{
    // Slicer logo
    this->SlicerLogo->SetImage( image_draftSlicerLogo,
                                image_draftSlicerLogo_width,
                                image_draftSlicerLogo_height,
                                image_draftSlicerLogo_pixel_size, 0, 0);
}



//---------------------------------------------------------------------------
void vtkSlicerLogoIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerLogoIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "SlicerLogo: " << this->GetSlicerLogo ( ) << "\n";

}
