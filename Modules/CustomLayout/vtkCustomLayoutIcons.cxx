#include "vtkObjectFactory.h"
#include "vtkCustomLayoutIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkCustomLayoutIcons );
vtkCxxRevisionMacro ( vtkCustomLayoutIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkCustomLayoutIcons::vtkCustomLayoutIcons ( )
{
  this->ContentAssignedIcon = vtkKWIcon::New ();
  this->ContentUnassignedIcon = vtkKWIcon::New ();
  
  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkCustomLayoutIcons::~vtkCustomLayoutIcons ( )
{

  if ( this->ContentAssignedIcon )
    {
    this->ContentAssignedIcon->Delete();
    this->ContentAssignedIcon = NULL;
    }
  if ( this->ContentUnassignedIcon )
    {
    this->ContentUnassignedIcon->Delete();
    this->ContentUnassignedIcon = NULL;
    }

}

//---------------------------------------------------------------------------
void vtkCustomLayoutIcons::AssignImageDataToIcons ( )
{

  this->ContentAssignedIcon->SetImage (image_ContentAssigned,
                              image_ContentAssigned_width,
                              image_ContentAssigned_height,
                              image_ContentAssigned_pixel_size,
                              image_ContentAssigned_length, 0 );
  this->ContentUnassignedIcon->SetImage (image_ContentUnassigned,
                              image_ContentUnassigned_width,
                              image_ContentUnassigned_height,
                              image_ContentUnassigned_pixel_size,
                              image_ContentUnassigned_length, 0 );
}



//---------------------------------------------------------------------------
void vtkCustomLayoutIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "CustomLayoutIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "ContentAssignedIcon: " << this->GetContentAssignedIcon ( ) << "\n";
    os << indent << "ContentUnassignedIcon: " << this->GetContentUnassignedIcon ( ) << "\n";
}
