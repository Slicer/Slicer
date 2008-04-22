#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWPushButton.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerXNATPermissionPrompterWidget.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerXNATPermissionPrompterWidget );
vtkCxxRevisionMacro (vtkSlicerXNATPermissionPrompterWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerXNATPermissionPrompterWidget::vtkSlicerXNATPermissionPrompterWidget()
{
  this->HostNameEntry = NULL;
  this->SetPromptMessage ("Please provide the following credentials for the data transfer.");
}

//---------------------------------------------------------------------------
vtkSlicerXNATPermissionPrompterWidget::~vtkSlicerXNATPermissionPrompterWidget()
{
  if ( this->HostNameEntry )
    {
    this->HostNameEntry->SetParent (NULL );
    this->HostNameEntry->Delete();
    this->HostNameEntry = NULL;
    }
}


//---------------------------------------------------------------------------
void vtkSlicerXNATPermissionPrompterWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerXNATPermissionPrompterWidget: " << this->GetClassName ( ) << "\n";
  os << indent << "HostNameEntry: " << this->GetHostNameEntry() << "\n";
}


//---------------------------------------------------------------------------
const char* vtkSlicerXNATPermissionPrompterWidget::GetHostFromWidget ( )
{
  if (this->GetHostNameEntry() != NULL )
    {
    return ( this->GetHostNameEntry()->GetWidget()->GetValue() );
    }
  return NULL;
}



//---------------------------------------------------------------------------
int vtkSlicerXNATPermissionPrompterWidget::Prompt( const char *message)
{
  if ( !this->GetRemember() || this->GetUsername()==NULL ||
       this->GetPassword()==NULL || this->GetHostName() == NULL )
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
        this->SetHostName ( this->GetHostFromWidget() );
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
void vtkSlicerXNATPermissionPrompterWidget::DestroyPrompter()
{

  if ( this->HostNameEntry )
    {
    this->HostNameEntry->SetParent (NULL );
    this->HostNameEntry->Delete();
    this->HostNameEntry = NULL;
    }
  this->Superclass::DestroyPrompter();
}


//---------------------------------------------------------------------------
void vtkSlicerXNATPermissionPrompterWidget::CreatePrompter(const char *messageText, const char *title)
{
  this->Superclass::CreatePrompter( messageText, title);
  
  //--- Resource for XNAT logo
  static const unsigned int  image_XNATLogo_width          = 100;
  static const unsigned int  image_XNATLogo_height         = 100;
  static const unsigned int  image_XNATLogo_pixel_size     = 3;
  static const unsigned long image_XNATLogo_length         = 3512;
  static const unsigned long image_XNATLogo_decoded_length = 30000;
  static const unsigned char image_XNATLogo[] = 
    "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"
    "9H+i/wEcz2za";
  this->GetLogoIcon()->SetImage ( image_XNATLogo,
                           image_XNATLogo_width,
                           image_XNATLogo_height,
                           image_XNATLogo_pixel_size,
                           image_XNATLogo_length, 0);
  this->GetLogoLabel()->SetImageToIcon ( this->LogoIcon );

  this->HostNameEntry = vtkKWEntryWithLabel::New();
  this->HostNameEntry->SetParent ( this->GetPromptDialog()->GetMessageDialogFrame() );
  this->HostNameEntry->Create();
  this->HostNameEntry->GetLabel()->SetText ("Host name: " );
  this->HostNameEntry->GetLabel()->SetBalloonHelpString ( "Enter name of the host." );
  this->HostNameEntry->GetWidget()->SetValue ( "" );
  this->HostNameEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->HostNameEntry->SetLabelWidth ( 20 );
  this->HostNameEntry->GetWidget()->SetWidth ( 30 );
  this->HostNameEntry->SetLabelPositionToLeft();

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  app->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                 this->HostNameEntry->GetWidgetName() );

}

