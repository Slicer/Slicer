#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerCheckButtonWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerCheckButtonWidget );
vtkCxxRevisionMacro ( vtkSlicerCheckButtonWidget, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
vtkSlicerCheckButtonWidget::vtkSlicerCheckButtonWidget()
{
  this->Icons = vtkSlicerCheckRadioButtonIcons::New();

}


//---------------------------------------------------------------------------
vtkSlicerCheckButtonWidget::~vtkSlicerCheckButtonWidget()
{
  if ( this->Icons )
    {
    this->Icons->Delete();
    this->Icons = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCheckButtonWidget::SetDisabled ( )
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
void vtkSlicerCheckButtonWidget::SetEnabled ( )
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
void vtkSlicerCheckButtonWidget::CreateWidget ( )
{
  if ( this->IsCreated() )
    {
    vtkErrorMacro ( <<this->GetClassName() << "already created");
    return;
    }
  
  // Call the superclass to create the entire widget
  this->Superclass::CreateWidget();
  
  this->SetEnabled();
  this->SetSelectImageToIcon ( this->Icons->GetSelectCheckIcon() );
  this->SelectedStateOff ( );
  this->IndicatorVisibilityOff ( );
  this->SetCompoundModeToLeft ( );
  this->SetText ( "");
  this->CustomIndicator = 1;
}

//---------------------------------------------------------------------------
void vtkSlicerCheckButtonWidget::SetCheckboxVisibility ( int visibility )
{
  if ( visibility )
    {
    this->IndicatorVisibilityOff ( );
    this->SetImageToIcon ( this->Icons->GetDeselectIcon ( ) );
    this->SetSelectImageToIcon ( this->Icons->GetSelectCheckIcon ( ) );
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
int vtkSlicerCheckButtonWidget::GetCheckboxVisibility ( )
{
  return ( this->CustomIndicator );
}

//---------------------------------------------------------------------------
void vtkSlicerCheckButtonWidget::CheckboxVisibilityOn ( )
{
    this->IndicatorVisibilityOff ( );
    this->SetImageToIcon ( this->Icons->GetDeselectIcon ( ) );
    this->SetSelectImageToIcon ( this->Icons->GetSelectCheckIcon ( ) );
    this->CustomIndicator = 1;
}

//---------------------------------------------------------------------------
void vtkSlicerCheckButtonWidget::CheckboxVisibilityOff ( )
{
    this->IndicatorVisibilityOff ( );
    this->SetConfigurationOption ("-image", "" );
    this->SetConfigurationOption ( "-selectimage", "" );
    this->CustomIndicator = 0;
}


//---------------------------------------------------------------------------
void vtkSlicerCheckButtonWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SlicerCheckButtonWidget: " << this->GetClassName ( ) << "\n";
  
  // class widgets
  os << indent << "Icons: " << this->GetIcons ( ) << "\n";
  os << indent << "CustomIndicator: " << this->GetCustomIndicator ( ) << "\n";

}



