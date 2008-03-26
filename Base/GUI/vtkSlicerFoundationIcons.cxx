
#include "vtkObjectFactory.h"
#include "vtkSlicerFoundationIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFoundationIcons );
vtkCxxRevisionMacro ( vtkSlicerFoundationIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::vtkSlicerFoundationIcons ( )
{
  this->GoIcon = vtkKWIcon::New();
  this->DoneIcon = vtkKWIcon::New();
  this->CameraIcon = vtkKWIcon::New();
  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerFoundationIcons::~vtkSlicerFoundationIcons ( )
{

  if ( this->GoIcon)
    {
    this->GoIcon->Delete();
    this->GoIcon = NULL;
    }
  if ( this->DoneIcon)
    {
    this->DoneIcon->Delete();
    this->DoneIcon = NULL;
    }
  if ( this->CameraIcon)
    {
    this->CameraIcon->Delete();
    this->CameraIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::AssignImageDataToIcons ( )
{
    // SliceVisibleIcon
  this->GoIcon->SetImage ( image_GoButton,
                           image_GoButton_width,
                           image_GoButton_height,
                           image_GoButton_pixel_size,
                           image_GoButton_length, 0);
  this->DoneIcon->SetImage ( image_DoneIndicator,
                             image_DoneIndicator_width,
                             image_DoneIndicator_height,
                             image_DoneIndicator_pixel_size,
                             image_DoneIndicator_length, 0);
  this->CameraIcon->SetImage ( image_CameraButton,
                               image_CameraButton_width,
                               image_CameraButton_height,
                               image_CameraButton_pixel_size,
                               image_CameraButton_length, 0);                               
}



//---------------------------------------------------------------------------
void vtkSlicerFoundationIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFoundationIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "GoIcon: " << this->GetGoIcon() << "\n";
    os << indent << "DoneIcon: " << this->GetDoneIcon() << "\n";
    os << indent << "CameraIcon: " << this->GetCameraIcon() << "\n";
}
