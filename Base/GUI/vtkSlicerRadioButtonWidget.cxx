#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerRadioButtonWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerRadioButtonWidget );
vtkCxxRevisionMacro ( vtkSlicerRadioButtonWidget, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
vtkSlicerRadioButtonWidget::vtkSlicerRadioButtonWidget()
{
  this->Icons = vtkSlicerCheckRadioButtonIcons::New();
}


//---------------------------------------------------------------------------
vtkSlicerRadioButtonWidget::~vtkSlicerRadioButtonWidget()
{
  if ( this->Icons )
    {
    this->Icons->Delete();
    this->Icons = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::SetDisabled ( )
{
  
  vtkKWIcon *icon = this->Icons->GetDisabledIcon();
  if ( icon )
    {
    this->SetImageToPixels ( icon->GetData(),
                             icon->GetWidth(),
                             icon->GetHeight(),
                             icon->GetPixelSize());
    }
  else
    {
    this->SetConfigurationOption ("-image", "");
    }

  this->SetStateToDisabled();
}


//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::SetEnabled ( )
{
  
  vtkKWIcon *icon = this->Icons->GetDeselectIcon();
  if ( icon )
    {
    this->SetImageToPixels ( icon->GetData(),
                             icon->GetWidth(),
                             icon->GetHeight(),
                             icon->GetPixelSize());
    }
  else
    {
    this->SetConfigurationOption ("-image", "");
    }
  this->SetStateToNormal();
}


//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::CreateWidget ( )
{
  if ( this->IsCreated() )
    {
    vtkErrorMacro ( <<this->GetClassName() << "already created");
    return;
    }
  // Call the superclass to create the entire widget
  this->Superclass::CreateWidget();

  this->SetEnabled();
  this->SetSelectImageToIcon ( this->Icons->GetSelectRadioIcon() );
  this->SelectedStateOff ( );
  this->IndicatorVisibilityOff();
  this->SetCompoundModeToLeft ( );
  this->SetText ( "");
  this->CustomIndicator = 1;
  
}

//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::SetRadioboxVisibility ( int visibility )
{
  if ( visibility )
    {
    this->IndicatorVisibilityOff ( );
    this->SetImageToIcon ( this->Icons->GetDeselectIcon ( ) );
    this->SetSelectImageToIcon ( this->Icons->GetSelectRadioIcon ( ) );
    this->CustomIndicator = 1;
    }
  else
    {
    this->IndicatorVisibilityOff ( );
    this->SetConfigurationOption ("-image", "" );
    this->SetConfigurationOption ( "-selectimage", "" );
    this->CustomIndicator = 0;
    }
}

//---------------------------------------------------------------------------
int vtkSlicerRadioButtonWidget::GetRadioboxVisibility ( )
{
  return ( this->CustomIndicator );
}

//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::RadioboxVisibilityOn ( )
{
    this->IndicatorVisibilityOff ( );
    this->SetImageToIcon ( this->Icons->GetDeselectIcon ( ) );
    this->SetSelectImageToIcon ( this->Icons->GetSelectRadioIcon ( ) );
    this->CustomIndicator = 1;
}

//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::RadioboxVisibilityOff ( )
{
    this->IndicatorVisibilityOff ( );
    this->SetConfigurationOption ("-image", "" );
    this->SetConfigurationOption ( "-selectimage", "" );
    this->CustomIndicator = 0;
}


//---------------------------------------------------------------------------
void vtkSlicerRadioButtonWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SlicerRadioButtonWidget: " << this->GetClassName ( ) << "\n";
  
  // class widgets
  os << indent << "Icons: " << this->GetIcons ( ) << "\n";
  os << indent << "CustomIndicator: " << this->GetCustomIndicator ( ) << "\n";
}



