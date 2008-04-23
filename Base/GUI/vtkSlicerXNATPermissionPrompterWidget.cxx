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
  this->SceneNameEntry = NULL;
  this->MRsessionIDEntry = NULL;
  this->SetPromptMessage ("Please provide the following credentials for the data transfer.");
  this->SceneName =NULL;
  this->MRsessionID=NULL;
}

//---------------------------------------------------------------------------
vtkSlicerXNATPermissionPrompterWidget::~vtkSlicerXNATPermissionPrompterWidget()
{
  if ( this->SceneNameEntry )
    {
    this->SceneNameEntry->SetParent ( NULL );
    this->SceneNameEntry->Delete();
    this->SceneNameEntry = NULL;
    }
  if ( this->HostNameEntry )
    {
    this->HostNameEntry->SetParent (NULL );
    this->HostNameEntry->Delete();
    this->HostNameEntry = NULL;
    }
  if ( this->MRsessionIDEntry )
    {
    this->MRsessionIDEntry->SetParent ( NULL );
    this->MRsessionIDEntry->Delete();
    this->MRsessionIDEntry = NULL;    
    }
}


//---------------------------------------------------------------------------
void vtkSlicerXNATPermissionPrompterWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerXNATPermissionPrompterWidget: " << this->GetClassName ( ) << "\n";
  os << indent << "HostNameEntry: " << this->GetHostNameEntry() << "\n";
  os << indent << "SceneNameEntry: " << this->GetSceneNameEntry() << "\n";
  os << indent << "MRsessionIDEntry: " << this->GetMRsessionIDEntry() << "\n";
}


//---------------------------------------------------------------------------
const char* vtkSlicerXNATPermissionPrompterWidget::GetHostFromWidget ( )
{
  if (this->GetHostNameEntry() != NULL )
    {
    return ( this->GetHostNameEntry()->GetWidget()->GetValue() );
    }
  return ("");
}

//---------------------------------------------------------------------------
const char* vtkSlicerXNATPermissionPrompterWidget::GetSceneNameFromWidget ( )
{
  if (this->GetSceneNameEntry() != NULL )
    {
      return ( this->GetSceneNameEntry()->GetWidget()->GetValue() );
    }
  return ("");
}

//---------------------------------------------------------------------------
const char* vtkSlicerXNATPermissionPrompterWidget::GetMRsessionIDFromWidget()
{
  if (this->GetMRsessionIDEntry() != NULL )
    {
      return ( this->GetMRsessionIDEntry()->GetWidget()->GetValue() );
    }
  return ("");
}


//---------------------------------------------------------------------------
int vtkSlicerXNATPermissionPrompterWidget::Prompt( const char *message, vtkMRMLScene *scene)
{
  if ( scene != NULL )
    {
    if ( !this->GetRemember() ||
         !strcmp (this->GetUsername(), "") ||
         !strcmp(this->GetPassword(), "") ||
         !strcmp(this->GetHostName(),"") ||
         !strcmp(this->GetMRsessionID(), "")  )
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
        //--- configure the widget with reasonable defaults.
        if ( !strcmp(this->GetHostName(), "") )
          {
          this->GetHostNameEntry()->GetWidget()->SetValue(this->GetHostName());
          }
        if ( !strcmp(scene->GetURL(), "" ) )
          {
          this->GetSceneNameEntry()->GetWidget()->SetValue(scene->GetURL());
          }
        if ( !strcmp(this->GetMRsessionID(), "") )
          {
          this->GetMRsessionIDEntry()->GetWidget()->SetValue(this->GetMRsessionID());
          }

        this->PromptDialog->Invoke();
        if ( this->PromptDialog->GetStatus() == vtkKWDialog::StatusCanceled )
          {
          this->DestroyPrompter();
          //--- return 0 if the transfer is cancelled.
          return 0;
          }
        else if ( this->PromptDialog->GetStatus() == vtkKWDialog::StatusOK )
          {
          this->SetUsername( this->GetUserFromWidget() );
          this->SetPassword ( this->GetPasswordFromWidget() );
          this->SetHostName ( this->GetHostFromWidget() );
          this->SetRemember ( this->GetRememberStatusFromWidget() );
          this->SetSceneName ( this->GetSceneNameFromWidget() );
          this->SetMRsessionID ( this->GetMRsessionIDFromWidget() );
          this->DestroyPrompter();        

          //--- prompt again if user didn't supply all information or cancel.
          if ( !strcmp (this->GetUsername(), "") ||
               !strcmp(this->GetPassword(), "") ||
               !strcmp(this->GetHostName(),"") ||
               !strcmp(this->GetMRsessionID(), "")  )
            {
            if ( (this->Prompt( "Please provide all missing information.", scene )) == 0 )
              {
              //--- transfer was cancelled
              return 0;
              }
            }
          }
        }
      this->DestroyPrompter();
      }
    //--- return 1 if everything looks complete.
    return 1;
    }
  return -1;
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
  if ( this->SceneNameEntry)
    {
    this->SceneNameEntry->SetParent ( NULL );
    this->SceneNameEntry->Delete();
    this->SceneNameEntry = NULL;
    }
  if ( this->MRsessionIDEntry)
    {
    this->MRsessionIDEntry->SetParent ( NULL );
    this->MRsessionIDEntry->Delete();
    this->MRsessionIDEntry = NULL;    
    }
  this->Superclass::DestroyPrompter();
}


//---------------------------------------------------------------------------
void vtkSlicerXNATPermissionPrompterWidget::CreatePrompter(const char *messageText, const char *title)
{
  this->Superclass::CreatePrompter( messageText, title);
  int h = this->GetPromptDialog()->GetMessageDialogFrame()->GetHeight();
  this->GetPromptDialog()->GetMessageDialogFrame()->SetHeight ( h+100);


/* 
 * Resource generated for file:
 *    XNATLogo.png (zlib, base64) (image file)
 */
  static const unsigned int  image_XNATLogo_width          = 146;
  static const unsigned int  image_XNATLogo_height         = 60;
  static const unsigned int  image_XNATLogo_pixel_size     = 3;
  static const unsigned long image_XNATLogo_length         = 5848;
  static const unsigned long image_XNATLogo_decoded_length = 26280;

  static const unsigned char image_XNATLogo[] = 
    "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"
    "R0DKPSqTGjNXWFSUxiEpOYxCQm6Yz8P8j9NFs=";

  this->SetSceneName ("");
  this->SetMRsessionID("");
  
  this->GetLogoIcon()->SetImage ( image_XNATLogo,
                           image_XNATLogo_width,
                           image_XNATLogo_height,
                           image_XNATLogo_pixel_size,
                           image_XNATLogo_length, 0);
  this->GetLogoLabel()->SetImageToIcon ( this->LogoIcon );

  this->HostNameEntry = vtkKWEntryWithLabel::New();
  this->HostNameEntry->SetParent ( this->GetPromptDialog()->GetTopFrame() );
  this->HostNameEntry->Create();
  this->HostNameEntry->GetLabel()->SetText ("Host name: " );
  this->HostNameEntry->GetLabel()->SetBalloonHelpString ( "Enter name of the host." );
  this->HostNameEntry->GetWidget()->SetValue ( this->GetHostName() );
  this->HostNameEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->HostNameEntry->SetLabelWidth ( 20 );
  this->HostNameEntry->GetWidget()->SetWidth ( 30 );
  this->HostNameEntry->SetLabelPositionToLeft();

  this->SceneNameEntry = vtkKWEntryWithLabel::New();
  this->SceneNameEntry->SetParent ( this->GetPromptDialog()->GetTopFrame() );
  this->SceneNameEntry->Create();
  this->SceneNameEntry->GetLabel()->SetText ("Scene name: " );
  this->SceneNameEntry->GetLabel()->SetBalloonHelpString ( "Entery the name of the scene.");
  this->SceneNameEntry->GetWidget()->SetValue ( this->GetSceneName() );
  this->SceneNameEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->SceneNameEntry->SetLabelWidth ( 20 );
  this->SceneNameEntry->GetWidget()->SetWidth ( 30 );
  this->SceneNameEntry->SetLabelPositionToLeft();

  this->MRsessionIDEntry = vtkKWEntryWithLabel::New();
  this->MRsessionIDEntry->SetParent ( this->GetPromptDialog()->GetTopFrame() );
  this->MRsessionIDEntry->Create();
  this->MRsessionIDEntry->GetLabel()->SetText ("MR session ID: " );
  this->MRsessionIDEntry->GetLabel()->SetBalloonHelpString ( "Enter the ID (XNAT Accession #) of the MR session to which data will be added." );
  this->MRsessionIDEntry->GetWidget()->SetValue ( this->GetMRsessionID() );
  this->MRsessionIDEntry->GetWidget()->SetCommandTriggerToReturnKeyAndFocusOut();
  this->MRsessionIDEntry->SetLabelWidth ( 20 );
  this->MRsessionIDEntry->GetWidget()->SetWidth ( 30 );
  this->MRsessionIDEntry->SetLabelPositionToLeft();

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  app->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                this->HostNameEntry->GetWidgetName());
  app->Script ( "pack %s -side top -padx 4 -pady 2 -expand y",
                this->MRsessionIDEntry->GetWidgetName());

}

