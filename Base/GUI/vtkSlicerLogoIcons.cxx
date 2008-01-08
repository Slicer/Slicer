

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
  // vertical format

    this->SlicerLogo->SetImage( image_Slicer3LogoHorizontal,
                                image_Slicer3LogoHorizontal_width,
                                image_Slicer3LogoHorizontal_height,
                                image_Slicer3LogoHorizontal_pixel_size,
                                image_Slicer3LogoHorizontal_length, 0);

  /*
    this->SlicerLogo->SetImage( image_Slicer3LogoHorizontalBeta,
                                image_Slicer3LogoHorizontalBeta_width,
                                image_Slicer3LogoHorizontalBeta_height,
                                image_Slicer3LogoHorizontalBeta_pixel_size,
                                image_Slicer3LogoHorizontalBeta_length, 0);
  */
/*
    this->SlicerLogo->SetImage( image_Slicer3LogoVerticalAlpha,
                                image_Slicer3LogoVerticalAlpha_width,
                                image_Slicer3LogoVerticalAlpha_height,
                                image_Slicer3LogoVerticalAlpha_pixel_size,
                                image_Slicer3LogoVerticalAlpha_length, 0);
*/
  // horizontal format
  /*
    this->SlicerLogo->SetImage( *image_Slicer3LogoHorizontal_sections,
                                image_Slicer3LogoHorizontal_width,
                                image_Slicer3LogoHorizontal_height,
                                image_Slicer3LogoHorizontal_pixel_size, 0, 0);
  */
}



//---------------------------------------------------------------------------
void vtkSlicerLogoIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerLogoIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "SlicerLogo: " << this->GetSlicerLogo ( ) << "\n";

}
