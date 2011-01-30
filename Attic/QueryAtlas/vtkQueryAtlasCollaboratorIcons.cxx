#include "vtkObjectFactory.h"
#include "vtkQueryAtlasCollaboratorIcons.h"
 
//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkQueryAtlasCollaboratorIcons );
vtkCxxRevisionMacro ( vtkQueryAtlasCollaboratorIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkQueryAtlasCollaboratorIcons::vtkQueryAtlasCollaboratorIcons ( )
{
    this->BrainInfoLogo = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkQueryAtlasCollaboratorIcons::~vtkQueryAtlasCollaboratorIcons ( )
{
    if ( this->BrainInfoLogo )
      {
      this->BrainInfoLogo->Delete ( );
      this->BrainInfoLogo = NULL;
      }
}

//---------------------------------------------------------------------------
void vtkQueryAtlasCollaboratorIcons::AssignImageDataToIcons ( )
{
  this->BrainInfoLogo->SetImage( image_BrainInfoLogo,
                                image_BrainInfoLogo_width,
                                image_BrainInfoLogo_height,
                                image_BrainInfoLogo_pixel_size,
                                image_BrainInfoLogo_length, 0);
}



//---------------------------------------------------------------------------
void vtkQueryAtlasCollaboratorIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "QueryAtlasCollaboratorIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "BrainInfoLogo: " << this->GetBrainInfoLogo ( ) << "\n";

}
