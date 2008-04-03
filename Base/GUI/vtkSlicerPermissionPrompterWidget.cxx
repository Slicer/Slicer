#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerPermissionPrompterWidget.h"

#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerPermissionPrompterWidget );
vtkCxxRevisionMacro (vtkSlicerPermissionPrompterWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerPermissionPrompterWidget::vtkSlicerPermissionPrompterWidget()
{
  this->PromptDialog = NULL;
  this->RememberCheck = NULL;
  this->LogoLabel = NULL;
  this->LogoIcon = NULL;
  this->UserNameEntry = NULL;
  this->PasswordEntry = NULL;
  this->Application = NULL;
  this->SetPromptMessage("Please provide the following credentials for the data transfer.");

}

//---------------------------------------------------------------------------
vtkSlicerPermissionPrompterWidget::~vtkSlicerPermissionPrompterWidget ( )
{
  this->DestroyPrompter();
  this->SetApplication (NULL );  
}

//---------------------------------------------------------------------------
int vtkSlicerPermissionPrompterWidget::GetRememberStatusFromWidget ( )
{
  if ( this->GetRememberCheck() != NULL )
    {
    return ( this->GetRememberCheck()->GetWidget()->GetSelectedState());
    }
  return (-1 );
}


//---------------------------------------------------------------------------
const char* vtkSlicerPermissionPrompterWidget::GetPasswordFromWidget()
{
  if ( this->GetPasswordEntry() != NULL )
    {
    return (this->GetPasswordEntry()->GetWidget()->GetValue());
    }
  return ( "" );
}


//---------------------------------------------------------------------------
const char* vtkSlicerPermissionPrompterWidget::GetUserFromWidget ( )
{
  if (this->GetUserNameEntry() != NULL )
    {
    return ( this->GetUserNameEntry()->GetWidget()->GetValue() );
    }
  return ("");
}


//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::SetLogoIcon( vtkKWIcon *icon )
{
  this->LogoLabel->SetImageToIcon ( icon );
}




//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::DestroyPrompter ()
{

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
  if ( this->PromptDialog )
    {
    this->PromptDialog->SetParent ( NULL );
    this->PromptDialog->Delete();
    this->PromptDialog = NULL;
    }

}

//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::CreatePrompter (const char *messageText, const char *title)
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );

  //--- dialog
  this->PromptDialog = vtkKWMessageDialog::New();
  this->PromptDialog->SetParent(app->GetApplicationGUI()->GetMainSlicerWindow()->GetViewFrame());
  this->PromptDialog->SetMasterWindow ( app->GetApplicationGUI()->GetMainSlicerWindow() );
  this->PromptDialog->SetStyleToOkCancel();
  this->PromptDialog->Create();
  this->PromptDialog->SetDisplayPositionToScreenCenter();
  this->PromptDialog->SetSize( 500, 300 );
  this->PromptDialog->SetTitle ( title );
  this->PromptDialog->SetText ( messageText );
    
  this->RememberCheck = vtkKWCheckButtonWithLabel::New();
  this->RememberCheck->SetParent (this->PromptDialog->GetMessageDialogFrame() );
  this->RememberCheck->Create();
  this->RememberCheck->GetWidget()->SetSelectedState (this->GetRemember() );
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
  this->LogoLabel->SetParent ( this->PromptDialog->GetMessageDialogFrame() );
  this->LogoLabel->Create();
  this->LogoLabel->SetImageToIcon ( this->LogoIcon );

  this->UserNameEntry = vtkKWEntryWithLabel::New();
  this->UserNameEntry->SetParent ( this->PromptDialog->GetMessageDialogFrame() );
  this->UserNameEntry->Create();
  this->UserNameEntry->GetLabel()->SetText ("User Name: " );
  this->UserNameEntry->GetLabel()->SetBalloonHelpString ( "Enter user name" );
  this->UserNameEntry->GetWidget()->SetValue ( this->GetUsername() );
  this->UserNameEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->UserNameEntry->SetLabelWidth ( 20 );
  this->UserNameEntry->GetWidget()->SetWidth ( 30 );
  this->UserNameEntry->SetLabelPositionToLeft();

  this->PasswordEntry = vtkKWEntryWithLabel::New();
  this->PasswordEntry->SetParent (  this->PromptDialog->GetMessageDialogFrame() );
  this->PasswordEntry->Create();
  this->PasswordEntry->GetLabel()->SetText ("Password: " );
  this->PasswordEntry->GetLabel()->SetBalloonHelpString ( "Enter password" );
  this->PasswordEntry->GetWidget()->SetValue ( this->GetPassword() );
  this->PasswordEntry->GetWidget()->PasswordModeOn();
  this->PasswordEntry->SetLabelWidth ( 20 );
  this->PasswordEntry->GetWidget()->SetWidth ( 30 );
  this->PasswordEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->PasswordEntry->SetLabelPositionToLeft();
  
  //--- pack it up.
  app->Script ( "pack %s %s -side top -padx 4 -pady 4 -expand n",
                 this->LogoLabel->GetWidgetName(),
                 this->RememberCheck->GetWidgetName());
  app->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                 this->UserNameEntry->GetWidgetName() );
  app->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                 this->PasswordEntry->GetWidgetName() );
}


//---------------------------------------------------------------------------
int vtkSlicerPermissionPrompterWidget::Prompt( const char *message )
{
  if ( !this->GetRemember() || this->GetUsername()==NULL || this->GetPassword()==NULL )
    {
    //--- create all widgets in prompt and customize message
    if ( message != NULL )
      {
      this->CreatePrompter(message, this->GetPromptTitle() );
      }
    else
      {
      this->CreatePrompter(this->GetPromptMessage(), this->GetPromptTitle() );
      }
    //--- Invoke and process result
    if ( this->PromptDialog != NULL )
      {
      this->PromptDialog->Invoke();
      if ( this->PromptDialog->GetStatus() == vtkKWDialog::StatusOK )
        {
        this->SetUsername( this->GetUserFromWidget() );
        this->SetPassword ( this->GetPasswordFromWidget() );
        this->SetRemember ( this->GetRememberStatusFromWidget() );
        this->DestroyPrompter();        
        if (  this->GetUsername() == "" || this->GetPassword() == "" )
          {
          //--- return -1 if not enough info was provided
          return -1;
          }
        }
      else if ( this->PromptDialog->GetStatus() == vtkKWDialog::StatusCanceled )
        {
        this->DestroyPrompter();
        //--- return 0 if the transfer is cancelled.
        return 0;
        }
      }
    this->DestroyPrompter();
    }
  //--- return 1 if everything looks complete.
  return 1;
}




//---------------------------------------------------------------------------
void vtkSlicerPermissionPrompterWidget::PrintSelf( ostream& os, vtkIndent indent )
{

  this->Superclass::PrintSelf (os, indent );
  os << indent << "vtkSlicerPermissionPrompterWidget: " << this->GetClassName ( ) << "\n";
  os << indent << "PromptDialog: " << this->GetPromptDialog ( ) << "\n";
  os << indent << "RememberCheck: " << this->GetRememberCheck ( ) << "\n";
  os << indent << "LogoLabel: " << this->GetLogoLabel ( ) << "\n";
  os << indent << "LogoIcon: " << this->GetLogoIcon ( ) << "\n";
  os << indent << "UserNameEntry: " << this->GetUserNameEntry ( ) << "\n";
  os << indent << "PasswordEntry: " << this->GetPasswordEntry ( ) << "\n";

}
