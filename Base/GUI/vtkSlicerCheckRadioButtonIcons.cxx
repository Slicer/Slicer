
#include "vtkObjectFactory.h"
#include "vtkSlicerCheckRadioButtonIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerCheckRadioButtonIcons );
vtkCxxRevisionMacro ( vtkSlicerCheckRadioButtonIcons, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerCheckRadioButtonIcons::vtkSlicerCheckRadioButtonIcons ( )
{
  this->DeselectIcon = vtkKWIcon::New();
  this->DisabledIcon = vtkKWIcon::New ( );
  this->SelectCheckIcon = vtkKWIcon::New();
  this->SelectRadioIcon = vtkKWIcon::New();
  this->AssignImageDataToIcons ();
}


//---------------------------------------------------------------------------
vtkSlicerCheckRadioButtonIcons::~vtkSlicerCheckRadioButtonIcons ( )
{
  if ( this->DeselectIcon )
    {
    this->DeselectIcon->Delete();
    this->DeselectIcon = NULL;
    }
  if ( this->DisabledIcon)
    {
    this->DisabledIcon->Delete();
    this->DisabledIcon = NULL;
    }
  if ( this->SelectCheckIcon )
    {
    this->SelectCheckIcon->Delete();
    this->SelectCheckIcon = NULL;
    }
  if ( this->SelectRadioIcon )
    {
    this->SelectRadioIcon->Delete();
    this->SelectRadioIcon = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCheckRadioButtonIcons::AssignImageDataToIcons ( )
{

  this->DeselectIcon->SetImage ( image_IndicatorOff,
                                 image_IndicatorOff_width,
                                 image_IndicatorOff_height,                                 
                                 image_IndicatorOff_pixel_size,
                                 image_IndicatorOff_length, 0);

  this->DisabledIcon->SetImage ( image_IndicatorDisabled,
                                 image_IndicatorDisabled_width,
                                 image_IndicatorDisabled_height,                                 
                                 image_IndicatorDisabled_pixel_size,
                                 image_IndicatorDisabled_length, 0);

  this->SelectCheckIcon->SetImage ( image_CheckIndicatorOn,
                                    image_CheckIndicatorOn_width,
                                    image_CheckIndicatorOn_height,
                                    image_CheckIndicatorOn_pixel_size,
                                    image_CheckIndicatorOn_length, 0);

  this->SelectRadioIcon->SetImage (image_RadioIndicatorOn,
                                   image_RadioIndicatorOn_width,
                                   image_RadioIndicatorOn_height,
                                   image_RadioIndicatorOn_pixel_size,
                                   image_RadioIndicatorOn_length, 0);
}



//---------------------------------------------------------------------------
void vtkSlicerCheckRadioButtonIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
     this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerCheckRadioButtonIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "DisabledIcon" << this->GetDisabledIcon( ) << "\n";
    os << indent << "DeselectIcon" << this->GetDeselectIcon( ) << "\n";
    os << indent << "SelectCheckIcon" << this->GetSelectCheckIcon( ) << "\n";
    os << indent << "SelectRadioIcon" << this->GetSelectRadioIcon( ) << "\n";
}

