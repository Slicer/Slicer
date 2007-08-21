
#include "vtkObjectFactory.h"
#include "vtkSlicerHelpIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerHelpIcons );
vtkCxxRevisionMacro ( vtkSlicerHelpIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerHelpIcons::vtkSlicerHelpIcons ( )
{
    this->HelpAndInformationIcon = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerHelpIcons::~vtkSlicerHelpIcons ( )
{

    if ( this->HelpAndInformationIcon )
      {
      this->HelpAndInformationIcon->Delete();
      this->HelpAndInformationIcon = NULL;
      }
}

//---------------------------------------------------------------------------
void vtkSlicerHelpIcons::AssignImageDataToIcons ( )
{

  this->HelpAndInformationIcon->SetImage( image_HelpAndInformation,
                                image_HelpAndInformation_width,
                                image_HelpAndInformation_height,
                                image_HelpAndInformation_pixel_size,
                                image_HelpAndInformation_length, 0);
}



//---------------------------------------------------------------------------
void vtkSlicerHelpIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerHelpIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "HelpAndInformationIcon: " << this->GetHelpAndInformationIcon ( ) << "\n";

}
