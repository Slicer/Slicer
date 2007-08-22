#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include "vtkSlicerPopUpHelpWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"

#include "vtkKWPushButton.h"
#include "vtkKWTopLevel.h"
#include "vtkKWText.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerPopUpHelpWidget );
vtkCxxRevisionMacro ( vtkSlicerPopUpHelpWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerPopUpHelpWidget::vtkSlicerPopUpHelpWidget ( )
{

  this->HelpButton = NULL;
  this->CloseButton = NULL;
  this->HelpWindow = NULL;
  this->HelpText = NULL;
  this->HelpTitle = NULL;
  this->HelpIcons = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerPopUpHelpWidget::~vtkSlicerPopUpHelpWidget ( )
{
  this->RemoveWidgetObservers();

  
  if ( this->CloseButton )
    {
    this->CloseButton->SetParent ( NULL );
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }
  if ( this->HelpButton )
    {
    this->HelpButton->SetParent ( NULL );
    this->HelpButton->Delete();
    this->HelpButton = NULL;
    }
  if ( this->HelpWindow )
    {
    this->HelpWindow->SetParent ( NULL );
    this->HelpWindow->Delete();
    this->HelpWindow = NULL;
    }
  if ( this->HelpText )
    {
    this->HelpText->SetParent ( NULL );
    this->HelpText->Delete();
    this->HelpText = NULL;
    }
  if ( this->HelpTitle )
    {
    this->HelpTitle->SetParent ( NULL );
    this->HelpTitle->Delete();
    this->HelpTitle = NULL;
    }
  if ( this->HelpIcons )
    {
    this->HelpIcons->Delete();
    this->HelpIcons = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerPopUpHelpWidget: " << this->GetClassName ( ) << "\n";
    os << indent << "HelpButton: " << this->GetHelpButton() << "\n";
    os << indent << "CloseButton: " << this->GetCloseButton() << "\n";
    os << indent << "HelpWindow: " << this->GetHelpWindow() << "\n";
    os << indent << "HelpText: " << this->GetHelpText() << "\n";
    os << indent << "HelpTitle: " << this->GetHelpTitle() << "\n";
    os << indent << "HelpIcons: " << this->GetHelpIcons() << "\n";

}


//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{
  // process id changed events
  if (this->HelpButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
    // pop up the help window
    this->DisplayHelpWindow();
    }
  if (this->CloseButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
    // close the help window
    this->WithdrawHelpWindow();
    }

} 



//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::AddWidgetObservers ( )
{
  this->HelpButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CloseButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::RemoveWidgetObservers ( )
{
  this->HelpButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CloseButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::Bind ( )
{
  if ( !this->IsCreated() )
    {
    return;
    }
  this->HelpButton->SetBinding ( "<ButtonPress>", this, "DisplayHelpWindow" );
  this->CloseButton->SetBinding ( "<ButtonPress>", this, "WithdrawHelpWindow" );
}

//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::UnBind ( )
{
  if (!this->IsCreated() )
    {
    return;
    }
  if ( this->HelpButton && this->HelpButton->IsCreated())
    {
    this->HelpButton->RemoveBinding ( "<ButtonPress>" );
    }
  if ( this->CloseButton && this->CloseButton->IsCreated() )
    {
    this->CloseButton->RemoveBinding ( "<ButtonPress>" );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::WithdrawHelpWindow ( )
{

  if ( !this->IsCreated() )
    {
    return;
    }
  this->HelpWindow->Withdraw();
}

//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::DisplayHelpWindow ( )
{
  
  if ( ! this->IsCreated() )
    {
    return;
    }
  this->HelpWindow->DeIconify();
  this->HelpWindow->Raise();
  this->Script ( "puts \"raising toplevel\""); 
}


//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::SetHelpText ( const char *text )
{
  this->HelpText->GetWidget()->SetText ( text );
}

//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::SetHelpTitle ( const char *title )
{
  this->HelpTitle->SetText ( title );
}


//---------------------------------------------------------------------------
void vtkSlicerPopUpHelpWidget::CreateWidget ( )
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
  vtkSlicerTheme *theme = NULL;
  vtkSlicerColor *color = NULL;
  if ( app )
    {
    theme = app->GetSlicerTheme();
    if ( theme)
      {
      color = theme->GetSlicerColors ( );
      }
    
    this->HelpIcons = vtkSlicerHelpIcons::New();

    this->HelpButton = vtkKWPushButton::New();
    this->HelpButton->SetParent (this->GetParent( )  );
    this->HelpButton->Create();
    this->HelpButton->SetReliefToFlat();
    this->HelpButton->SetBorderWidth ( 0 );
    this->HelpButton->SetImageToIcon ( this->HelpIcons->GetHelpAndInformationIcon() );

    this->HelpWindow = vtkKWTopLevel::New();
    this->HelpWindow->SetMasterWindow ( this->GetParent ( ) );
    this->HelpWindow->SetApplication ( app );
    this->HelpWindow->Create();
    this->HelpWindow->SetBorderWidth ( 2 );
    this->HelpWindow->SetReliefToFlat ( );
    this->HelpWindow->SetDisplayPositionToPointer();
    this->HelpWindow->SetTitle("Slicer Information");
    this->HelpWindow->SetSize ( 300, 350 );
    this->HelpWindow->Withdraw();
    this->HelpWindow->SetDeleteWindowProtocolCommand ( this, "WithdrawHelpWindow" );

    this->CloseButton = vtkKWPushButton::New();
    this->CloseButton->SetParent ( this->HelpWindow );
    this->CloseButton->Create();
    this->CloseButton->SetText ( "close" );
    if ( color )
      {
      // try a little styling here;
      // TODO: move to theme if good.
      this->CloseButton->SetBackgroundColor ( color->LightestGrey );
      }
  
    this->HelpText = vtkKWTextWithScrollbars::New();
    this->HelpText->SetParent ( this->HelpWindow );
    this->HelpText->Create();
    this->HelpText->HorizontalScrollbarVisibilityOn();
    this->HelpText->VerticalScrollbarVisibilityOn();
    this->HelpText->GetWidget()->SetReliefToGroove();
    this->HelpText->GetWidget()->QuickFormattingOn();
    this->HelpText->GetWidget()->SetWrapToWord();

    this->HelpTitle = vtkKWLabel::New();
    this->HelpTitle->SetParent ( this->HelpWindow );
    this->HelpTitle->Create();
    this->HelpTitle->SetJustificationToCenter();
    this->HelpTitle->SetHeight ( 2 );    

    // pack all but HelpButton, which gets packed into another widget.
    this->Script ("grid %s -row 0 -column 0 -sticky ew -padx 0 -pady 0", this->HelpTitle->GetWidgetName() );
    this->Script ("grid %s -row 1 -column 0 -sticky news -padx 2 -pady 1", this->HelpText->GetWidgetName() );
    this->Script ("grid %s -row 2 -column 0 -padx 2 -pady 4", this->CloseButton->GetWidgetName() );

    this->Script ( "grid rowconfigure %s 0 -weight 0", this->HelpWindow->GetWidgetName() );
    this->Script ( "grid rowconfigure %s 1 -weight 1", this->HelpWindow->GetWidgetName() );
    this->Script ( "grid rowconfigure %s 2 -weight 0", this->HelpWindow->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 0 -weight 1", this->HelpWindow->GetWidgetName() );


    this->Script ("pack %s -side top -expand n -anchor c -padx 0 -pady 0", this->HelpButton->GetWidgetName() );
    }
  this->Bind();
}



