#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkFetchMITagViewWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMITagViewWidget );
vtkCxxRevisionMacro ( vtkFetchMITagViewWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMITagViewWidget::vtkFetchMITagViewWidget ( )
{

  this->CloseButton = NULL;
  this->TagViewWindow = NULL;
  this->TitleLabel = NULL;
  this->TextBox = NULL;
}


//---------------------------------------------------------------------------
vtkFetchMITagViewWidget::~vtkFetchMITagViewWidget ( )
{
  if ( this->CloseButton )
    {
    this->CloseButton->SetParent ( NULL );
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }
  if ( this->TitleLabel )
    {
    this->TitleLabel->SetParent ( NULL );
    this->TitleLabel->Delete();
    this->TitleLabel = NULL;
    }
  if ( this->TextBox )
    {
    this->TextBox->SetParent ( NULL );
    this->TextBox->Delete();
    this->TextBox = NULL;
    }
  if ( this->TagViewWindow )
    {
    this->TagViewWindow->SetParent ( NULL );
    this->TagViewWindow->Delete();
    this->TagViewWindow = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkFetchMITagViewWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "CloseButton: " << this->GetCloseButton() << "\n";
    os << indent << "TagViewWindow: " << this->GetTagViewWindow() << "\n";
    os << indent << "TitleLabel: " << this->GetTitleLabel() << "\n";
    os << indent << "TextBox: " << this->GetTextBox () << "\n";

}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::Bind ( )
{
  if ( !this->IsCreated() )
    {
    return;
    }
  this->CloseButton->SetBinding ( "<ButtonPress>", this, "DestroyTagViewWindow" );
}

//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::UnBind ( )
{
  if (!this->IsCreated() )
    {
    return;
    }
  if ( this->CloseButton && this->CloseButton->IsCreated() )
    {
    this->CloseButton->RemoveBinding ( "<ButtonPress>" );
    }
}

//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::DestroyTagViewWindow ( )
{

  if ( !this->IsCreated() )
    {
    return;
    }
  this->TagViewWindow->Withdraw();
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::DisplayTagViewWindow ( )
{
  
  if ( ! this->IsCreated() )
    {
    return;
    }
  this->TagViewWindow->DeIconify();
  this->TagViewWindow->Raise();
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::SetTagText ( const char *text )
{
  this->TextBox->GetWidget()->SetText ( text );
}

//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::SetTagTitle ( const char *title )
{
  this->TitleLabel->SetText ( title );
}


//---------------------------------------------------------------------------
void vtkFetchMITagViewWidget::CreateWidget ( )
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

    this->TagViewWindow = vtkKWTopLevel::New();
    this->TagViewWindow->SetMasterWindow ( this->GetParent ( ) );
    this->TagViewWindow->SetApplication ( app );
    this->TagViewWindow->Create();
    this->TagViewWindow->SetBorderWidth ( 2 );
    this->TagViewWindow->SetReliefToFlat ( );
    this->TagViewWindow->SetDisplayPositionToMasterWindowCenterFirst();
    this->TagViewWindow->SetTitle("Tag Viewer");
    this->TagViewWindow->SetSize ( 300, 350 );
    this->TagViewWindow->Withdraw();
    this->TagViewWindow->SetDeleteWindowProtocolCommand ( this, "DestroyTagViewWindow" );

    this->CloseButton = vtkKWPushButton::New();
    this->CloseButton->SetParent ( this->TagViewWindow );
    this->CloseButton->Create();
    this->CloseButton->SetText ( "close" );
  
    this->TitleLabel = vtkKWLabel::New();
    this->TitleLabel->SetParent ( this->TagViewWindow );
    this->TitleLabel->Create();
    this->TitleLabel->SetJustificationToCenter();
    this->TitleLabel->SetHeight ( 2 );    

    this->TextBox = vtkKWTextWithScrollbars::New ( );
    this->TextBox->SetParent ( this->TagViewWindow );
    this->TextBox->Create ( );
    this->TextBox->SetWidth (30 );
    this->TextBox->HorizontalScrollbarVisibilityOn();
    this->TextBox->VerticalScrollbarVisibilityOn();
    this->TextBox->GetWidget()->SetReliefToGroove();
    this->TextBox->GetWidget()->QuickFormattingOn();
    this->TextBox->GetWidget()->SetWrapToWord();

    // pack all but HelpButton, which gets packed into another widget.
    this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 2", this->TitleLabel->GetWidgetName() );
    this->Script ("pack %s -side top -expand y -fill both -anchor c -padx 2 -pady 4", this->TextBox->GetWidgetName() );
    this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 2", this->CloseButton->GetWidgetName() );
    this->Bind();
}



