
// vtkSlicer includes
#include "vtkSlicerApplication.h"
#include "vtkSlicerWaitMessageWidget.h"
#include "vtkSlicerFoundationIcons.h"

// KWWidgets includes
#include "vtkKWLabel.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"

// VTK includes
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerWaitMessageWidget );
vtkCxxRevisionMacro ( vtkSlicerWaitMessageWidget, "$Revision$");


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
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    app->Script ( "grab release %s", this->MessageWindow->GetWidgetName() );
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
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if ( app )
    {
    app->Script ( "grab %s", this->MessageWindow->GetWidgetName() );
    app->ProcessIdleTasks();
    }
  this->Script ("update idletasks");  
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
//      this->MessageWindow->ModalOn();
      int px, py, sx, sy;
      vtkKWTkUtilities::GetWidgetCoordinates(this->GetParent(), &px, &py);
      vtkKWTkUtilities::GetWidgetSize(this->GetParent(), &sx, &sy);
      this->MessageWindow->SetPosition ( px + sx/2 - 300, py + sy/2 - 60) ;
      this->MessageWindow->SetBorderWidth ( 1 );
      this->MessageWindow->SetReliefToFlat ( );
      this->MessageWindow->SetTitle("Slicer operation in progress");
      this->MessageWindow->SetSize ( 600, 120 );
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



