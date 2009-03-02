#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerWaitMessageWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFoundationIcons.h"

#include "vtkKWLabel.h"
#include "vtkKWTopLevel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerWaitMessageWidget );
vtkCxxRevisionMacro ( vtkSlicerWaitMessageWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerWaitMessageWidget::vtkSlicerWaitMessageWidget ( )
{

  this->MessageWindow = NULL;
  this->TextLabel = NULL;
  this->ImageLabel = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerWaitMessageWidget::~vtkSlicerWaitMessageWidget ( )
{
  if ( this->TextLabel )
    {
    this->TextLabel->SetParent ( NULL );
    this->TextLabel->Delete();
    this->TextLabel = NULL;
    }
  if ( this->ImageLabel )
    {
    this->ImageLabel->SetParent ( NULL );
    this->ImageLabel->Delete();
    this->ImageLabel = NULL;
    }
  if ( this->MessageWindow )
    {
    this->MessageWindow->SetParent ( NULL );
    this->MessageWindow->Delete();
    this->MessageWindow = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerWaitMessageWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "MessageWindow: " << this->GetMessageWindow() << "\n";
    os << indent << "ImageLabel: " << this->GetImageLabel() << "\n";
    os << indent << "TextLabel: " << this->GetTextLabel() << "\n";
}


//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::WithdrawWindow ( )
{

  if ( !this->IsCreated() )
    {
    return;
    }
  this->MessageWindow->Withdraw();
}

//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::DisplayWindow ( )
{
  
  if ( ! this->IsCreated() )
    {
    return;
    }
  this->MessageWindow->DeIconify();
  this->MessageWindow->Raise();
}


//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::SetText ( const char *text )
{
  this->GetTextLabel()->SetText ( text );
}



//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::ResetText ( )
{
  this->TextLabel->SetText ( "Please wait... this operation may take some time." );
}



//---------------------------------------------------------------------------
void vtkSlicerWaitMessageWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
    if ( appGUI )
      {
      this->MessageWindow = vtkKWTopLevel::New();
      this->MessageWindow->SetMasterWindow ( this->GetParent ( ) );
      this->MessageWindow->SetApplication ( app );
      this->MessageWindow->Create();
      this->MessageWindow->SetBorderWidth ( 2 );
      this->MessageWindow->SetReliefToFlat ( );
      this->MessageWindow->SetDisplayPositionToPointer();
      this->MessageWindow->SetTitle("Slicer operation in progress");
      this->MessageWindow->SetSize ( 350, 300 );
      this->MessageWindow->HideDecorationOn();
      this->MessageWindow->Withdraw();
      this->MessageWindow->SetDeleteWindowProtocolCommand ( this, "WithdrawWindow" );

      this->ImageLabel = vtkKWLabel::New();
      this->ImageLabel->SetParent ( this->MessageWindow );
      this->ImageLabel->Create();
      this->ImageLabel->SetBorderWidth ( 0 );
      this->ImageLabel->SetReliefToFlat ( );
      this->ImageLabel->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerWaitIcon() );

      this->TextLabel = vtkKWLabel::New();
      this->TextLabel->SetParent ( this->MessageWindow );
      this->TextLabel->Create();
      this->TextLabel->SetBorderWidth ( 0 );
      this->TextLabel->SetReliefToFlat ( );
      this->TextLabel->SetText ( "Please wait... this operation may take some time." );
  
      this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 20", this->ImageLabel->GetWidgetName() );
      this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 0", this->TextLabel->GetWidgetName() );
      }
    }
}



