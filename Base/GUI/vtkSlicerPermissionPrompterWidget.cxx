#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerPermissionPrompterWidget.h"

#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerPermissionPrompterWidget );
vtkCxxRevisionMacro ( vtkSlicerPermissionPrompterWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerPermissionPrompterWidget::vtkSlicerPermissionPrompterWidget()
{
  this->PromptWindow = NULL;
  this->OKButton = NULL;
  this->CancelButton = NULL;
  this->RememberCheck = NULL;
  this->LogoLabel = NULL;
  this->LogoIcon = NULL;
  this->UserNameEntry = NULL;
  this->PasswordEntry = NULL;
  this->PermissionInfo = NULL;

}

//---------------------------------------------------------------------------
vtkSlicerPermissionPrompterWidget::~vtkSlicerPermissionPrompterWidget ( )
{
  if ( this->OKButton )
    {
    this->OKButton->SetParent (NULL );
    this->OKButton->Delete();
    this->OKButton = NULL;
    }
  if ( this->CancelButton )
    {
    this->CancelButton->SetParent ( NULL);
    this->CancelButton->Delete();
    this->CancelButton = NULL;
    }
  if ( this->RememberCheck )
    {
    this->RememberCheck->SetParent ( NULL );
    this->RememberCheck->Delete();
    this->RememberCheck = NULL;
    }
  if ( this->LogoLabel )
    {
    this->LogoLabel->SetParent ( NULL );
    this->LogoLabel->Delete();
    this->LogoLabel = NULL;
    }
  if ( this->LogoIcon )
    {
    this->LogoIcon->Delete();
    this->LogoIcon = NULL;
    }
  if ( this->UserNameEntry )
    {
    this->UserNameEntry->SetParent ( NULL );
    this->UserNameEntry->Delete();
    this->UserNameEntry = NULL;
    }
  if ( this->PasswordEntry )
    {
    this->PasswordEntry->SetParent ( NULL );
    this->PasswordEntry->Delete();
    this->PasswordEntry = NULL;
    }
  if ( this->PromptWindow )
    {
    this->PromptWindow->SetParent ( NULL );
    this->PromptWindow->Delete();
    this->PromptWindow = NULL;
    }

  this->SetPermissionInfo (NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf (os, indent );
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerPermissionPrompterWidget: " << this->GetClassName ( ) << "\n";
    
  os << indent << "PromptWindow: " << this->GetPromptWindow ( ) << "\n";
  os << indent << "OKButton: " << this->GetOKButton ( ) << "\n";
  os << indent << "CancelButton: " << this->GetCancelButton ( ) << "\n";
  os << indent << "RememberCheck: " << this->GetRememberCheck ( ) << "\n";
  os << indent << "LogoLabel: " << this->GetLogoLabel ( ) << "\n";
  os << indent << "LogoIcon: " << this->GetLogoIcon ( ) << "\n";
  os << indent << "UserNameEntry: " << this->GetUserNameEntry ( ) << "\n";
  os << indent << "PasswordEntry: " << this->GetPasswordEntry ( ) << "\n";
  os << indent << "PermissionInfo: " << this->GetPermissionInfo ( ) << "\n";
}




//---------------------------------------------------------------------------
const char* vtkSlicerPermissionPrompterWidget::GetPassword()
{
  return (this->GetPasswordEntry()->GetWidget()->GetValue());
}

//---------------------------------------------------------------------------
const char* vtkSlicerPermissionPrompterWidget::GetUser ( )
{
  return ( this->GetUserNameEntry()->GetWidget()->GetValue() );
}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::SetLogoIcon( vtkKWIcon *icon )
{
  this->LogoLabel->SetImageToIcon ( icon );
}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::SetTitle ( const char *title )
{
  if ( this->PromptWindow )
    {
    this->PromptWindow->SetTitle ( title );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::AddWidgetObservers()
{
  this->OKButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RememberCheck->GetWidget()->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UserNameEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->PasswordEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::RemoveWidgetObservers()
{
  this->OKButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RememberCheck->GetWidget()->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UserNameEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->PasswordEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::ProcessWidgetEvents( vtkObject *caller, unsigned long event, void *callData)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );

  if ( cb == this->RememberCheck->GetWidget() && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    //--- Set MRML flag so we prompt at each data transfer.
    this->PermissionInfo->SetRemember ( this->RememberCheck->GetWidget()->GetSelectedState() );
    }

  if ( e == this->UserNameEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
    {
    //this->PermissionInfo->SetUserName ( this->UserNameEntry->GetWidget()->GetValue() );
    }
  else if ( e == this->PasswordEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
    {
    //this->PermissionInfo->SetPassword ( this->PasswordEntry->GetWidget()->GetValue() );
    }

  if ( b == this->OKButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->PermissionInfo->SetUserName ( this->UserNameEntry->GetWidget()->GetValue() );    
    this->PermissionInfo->SetPassword ( this->PasswordEntry->GetWidget()->GetValue() );
    this->HidePermissionPrompter();
    }
  else if ( b == this->CancelButton && event == vtkKWPushButton::InvokedEvent )
    {
//    this->PermissionInfo->SetUserName ( "" );    
//    this->PermissionInfo->SetPassword ( "" );
    this->HidePermissionPrompter();
    }

  return;
}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::CreateWidget()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );

  //--- top level
  this->PromptWindow = vtkKWTopLevel::New();
  this->PromptWindow->SetApplication ( app );
  this->PromptWindow->SetTitle ( "Permission Prompt" );
  this->PromptWindow->Create();
  this->PromptWindow->SetBorderWidth ( 2 );
  this->PromptWindow->SetReliefToFlat();
  this->PromptWindow->SetDisplayPositionToPointer();
  this->PromptWindow->SetSize ( 500, 300 );
  this->PromptWindow->SetMinimumSize ( 500, 300 );
  this->PromptWindow->Withdraw();
  this->PromptWindow->SetDeleteWindowProtocolCommand ( this, "HidePermissionPrompter" );
    
  this->OKButton = vtkKWPushButton::New();
  this->OKButton->SetParent ( this->PromptWindow );
  this->OKButton->Create();
  this->OKButton->SetText ( "OK");
  this->OKButton->SetBalloonHelpString ( "Use the values entered and close the window." );

  this->CancelButton = vtkKWPushButton::New();
  this->CancelButton->SetParent ( this->PromptWindow );
  this->CancelButton->Create();
  this->CancelButton->SetText ( "Cancel ");
  this->CancelButton->SetBalloonHelpString ( "Don't use values entered, close window." );

  this->RememberCheck = vtkKWCheckButtonWithLabel::New();
  this->RememberCheck->SetParent (this->PromptWindow );
  this->RememberCheck->Create();
  this->RememberCheck->GetWidget()->SetSelectedState (1);
  this->RememberCheck->GetLabel()->SetText ( "Remember this user name and password for this session." );
  
  this->LogoIcon = vtkKWIcon::New();
  //--- Resource generated for file:
  //--- LogoBlank.png (zlib, base64) (image file)
  static const unsigned int  image_LogoBlank_width          = 100;
  static const unsigned int  image_LogoBlank_height         = 100;
  static const unsigned int  image_LogoBlank_pixel_size     = 3;
  static const unsigned long image_LogoBlank_length         = 72;
  static const unsigned long image_LogoBlank_decoded_length = 30000;
  static const unsigned char image_LogoBlank[] = 
    "eNrtwTEBAAAAwqD+qWcJT6AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD4Gu3vBnQ"
    "==";
  this->LogoIcon->SetImage ( image_LogoBlank,
                           image_LogoBlank_width,
                           image_LogoBlank_height,
                           image_LogoBlank_pixel_size,
                           image_LogoBlank_length, 0);

  this->LogoLabel = vtkKWLabel::New();
  this->LogoLabel->SetParent ( this->PromptWindow );
  this->LogoLabel->Create();
  this->LogoLabel->SetImageToIcon ( this->LogoIcon );

  this->UserNameEntry = vtkKWEntryWithLabel::New();
  this->UserNameEntry->SetParent ( this->PromptWindow );
  this->UserNameEntry->Create();
  this->UserNameEntry->GetLabel()->SetText ("User Name: " );
  this->UserNameEntry->GetLabel()->SetBalloonHelpString ( "Enter user name" );
  this->UserNameEntry->GetWidget()->SetValue ( "" );
  this->UserNameEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->UserNameEntry->SetLabelWidth ( 20 );
  this->UserNameEntry->GetWidget()->SetWidth ( 30 );
  this->UserNameEntry->SetLabelPositionToLeft();

  this->PasswordEntry = vtkKWEntryWithLabel::New();
  this->PasswordEntry->SetParent ( this->PromptWindow );
  this->PasswordEntry->Create();
  this->PasswordEntry->GetLabel()->SetText ("Password: " );
  this->PasswordEntry->GetLabel()->SetBalloonHelpString ( "Enter password" );
  this->PasswordEntry->GetWidget()->SetValue ( "" );
  this->PasswordEntry->GetWidget()->PasswordModeOn();
  this->PasswordEntry->SetLabelWidth ( 20 );
  this->PasswordEntry->GetWidget()->SetWidth ( 30 );
  this->PasswordEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->PasswordEntry->SetLabelPositionToLeft();
  
  //--- pack it up.
  this->Script ( "pack %s %s -side top -padx 4 -pady 4 -expand n",
                 this->LogoLabel->GetWidgetName(),
                 this->RememberCheck->GetWidgetName());
  this->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                 this->UserNameEntry->GetWidgetName() );
  this->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                 this->PasswordEntry->GetWidgetName() );

  //--- set 'built' flag true
  this->Built = true;
}


//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::DisplayPermissionPrompter()
{
   if (! this->Built )
    {
    return;
    }
   vtkDebugMacro("vtkSlicerPermissionPrompterWidget: Displaying PermissionPrompter.");
  this->PromptWindow->DeIconify();
  this->PromptWindow->Raise();
  vtkDebugMacro("vtkSlicerPermissionPrompterWidget: DONE displaying PermissionPrompter.");
}


//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::HidePermissionPrompter()
{
    if ( ! this->Built )
    {
    return;
    }
    vtkDebugMacro("vtkSlicerPermissionPrompter: Withdrawing PermissionPrompter.");
  this->PromptWindow->Withdraw();
  vtkDebugMacro("vtkSlicerPermissionPrompter: Done withdrawing Permission Prompter.");
}
