

#include "vtkObjectFactory.h"
#include "vtkSlicerBaseAcknowledgementLogoIcons.h"
 
//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerBaseAcknowledgementLogoIcons );
vtkCxxRevisionMacro ( vtkSlicerBaseAcknowledgementLogoIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerBaseAcknowledgementLogoIcons::vtkSlicerBaseAcknowledgementLogoIcons ( )
{
    this->NACLogo = vtkKWIcon::New ( );
    this->BIRNLogo = vtkKWIcon::New ( );
    this->NAMICLogo = vtkKWIcon::New ( );
    this->NCIGTLogo = vtkKWIcon::New ( );    
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerBaseAcknowledgementLogoIcons::~vtkSlicerBaseAcknowledgementLogoIcons ( )
{

    if ( this->NAMICLogo ) {
            this->NAMICLogo->Delete ( );
            this->NAMICLogo = NULL;
        }
    if ( this->NACLogo ) {
            this->NACLogo->Delete ( );
            this->NACLogo = NULL;
        }
    if ( this->NCIGTLogo ) {
            this->NCIGTLogo->Delete ( );
            this->NCIGTLogo = NULL;
        }
    if ( this->BIRNLogo ) {
            this->BIRNLogo->Delete ( );
            this->BIRNLogo = NULL;
        }
}

//---------------------------------------------------------------------------
void vtkSlicerBaseAcknowledgementLogoIcons::AssignImageDataToIcons ( )
{
  this->NAMICLogo->SetImage( image_NAMIC,
                                image_NAMIC_width,
                                image_NAMIC_height,
                                image_NAMIC_pixel_size,
                                image_NAMIC_length, 0);
  this->NCIGTLogo->SetImage( image_NCIGT,
                                image_NCIGT_width,
                                image_NCIGT_height,
                                image_NCIGT_pixel_size,
                                image_NCIGT_length, 0);
  this->NACLogo->SetImage( image_NAC,
                                image_NAC_width,
                                image_NAC_height,
                                image_NAC_pixel_size,
                                image_NAC_length, 0);
  this->BIRNLogo->SetImage( image_BIRN,
                                image_BIRN_width,
                                image_BIRN_height,
                                image_BIRN_pixel_size,
                                image_BIRN_length, 0);

}



//---------------------------------------------------------------------------
void vtkSlicerBaseAcknowledgementLogoIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerBaseAcknowledgementLogoIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "NAMICLogo: " << this->GetNAMICLogo ( ) << "\n";
    os << indent << "NACLogo: " << this->GetNACLogo ( ) << "\n";
    os << indent << "NCIGTLogo: " << this->GetNCIGTLogo ( ) << "\n";
    os << indent << "BIRNLogo: " << this->GetBIRNLogo ( ) << "\n";

}
