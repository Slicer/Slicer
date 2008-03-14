#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerDataTransferWidget.h"

//widgets
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWTopLevel.h"
#include "vtkKWFrame.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWText.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWMessageDialog.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerDataTransferIcons.h"

#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataTransferWidget);
vtkCxxRevisionMacro (vtkSlicerDataTransferWidget, "$Revision: 1.0 $");
//---------------------------------------------------------------------------

vtkSlicerDataTransferWidget::vtkSlicerDataTransferWidget(void)
  {
    // data transfer container and its widgets
    this->TransferID = 0;
    this->DataTransferFrame = NULL;
    this->URILabel = NULL;
    this->TransferTypeLabel = NULL;
    this->TransferStatusLabel = NULL;
    this->CancelButton = NULL;
    this->DeleteButton = NULL;
    this->DataTransferIcons = NULL;
    this->InformationButton = NULL;  
    // pop-up information frame
    this->InformationTopLevel = NULL;
    this->InformationFrame = NULL;
    this->InformationText = NULL;
    this->InformationCloseButton = NULL;
    this->DataTransfer = NULL;
    this->TimerCount = 0;
    this->TimerSteps = 8;
    this->TimerRunning = 0;
    this->CacheManager = NULL;
  }


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::StartTransferTimer ()
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app )
    {
    vtkDebugMacro ("vtkSlicerDataTransferWidget: Turning on TransferTimer...");
    this->SetTimerID ( vtkKWTkUtilities::CreateTimerHandler ( app->GetMainInterp(), 10, this, "UpdateTransferFeedback") );
    }
    this->TimerRunning = 1;
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::KillTransferTimer ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( (app != NULL) && (this->GetTimerID() != NULL) )
    {
    vtkDebugMacro ("vtkSlicerDataTransferWidget: Killing TransferTimer...");
    vtkKWTkUtilities::CancelTimerHandler ( app->GetMainInterp(), this->GetTimerID() );
    }
  this->SetTimerID ("" );
  this->TimerRunning = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateTransferFeedback()
{


  vtkDebugMacro ("vtkSlicerDataTransferWidget: Updating transfer feedback.");
  //--- if the transfer has been completed in an asynchronous
  //--- thread, its status has been modified without triggering
  //--- an event. So always check this here to capture
  //--- completed or canceled events and kill the timer.
  if ( this->DataTransfer == NULL)
    {
    return;
    }
  if ( this->DataTransfer->GetTransferStatus() == vtkDataTransfer::Completed)
    {
    this->TimerCount = 0;
    this->UpdateWidget();
    }
  else
    {
    //--- todo: yuk, make this less hardcoded to 8 icons...
    switch ( this->TimerCount )
      {
      case 0:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing0Icon());
        break;
      case 1:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing1Icon());
        break;
      case 2:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing2Icon());
        break;
      case 3:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing3Icon());
        break;
      case 4:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing4Icon());
        break;
      case 5:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing5Icon());
        break;
      case 6:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing6Icon());
        break;
      case 7:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoing7Icon());
        break;
      default:
        this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoingIcon());
        break;
      }
    if ( this->TimerCount == 7 )
      {
      this->TimerCount = 0;
      }
    else
      {
      this->TimerCount++;
      }

    //--- update the GUI
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
    if ( app != NULL )
      {
//      app->ProcessIdleTasks();
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::PrintSelf (ostream& os, vtkIndent indent)
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerDataTransferWidget: " << this->GetClassName ( ) << "\n";
  os << indent << "DataTransferFrame: " << this->GetDataTransferFrame ( ) << "\n";
  os << indent << "URILabel: " << this->GetURILabel ( ) << "\n";
  os << indent << "TransferTypeLabel: " << this->GetTransferTypeLabel ( ) << "\n";
  os << indent << "TransferStatusLabel: " << this->GetTransferStatusLabel ( ) << "\n";
  os << indent << "CancelButton: " << this->GetCancelButton ( ) << "\n";
  os << indent << "DeleteButton: " << this->GetDeleteButton ( ) << "\n";
  os << indent << "DataTransferIcons: " << this->GetDataTransferIcons ( ) << "\n";
  os << indent << "InformationButton: " << this->GetInformationButton ( ) << "\n";
  os << indent << "InformationTopLevel: " << this->GetInformationTopLevel ( ) << "\n";
  os << indent << "InformationFrame: " << this->GetInformationFrame ( ) << "\n";
  os << indent << "InformationText: " << this->GetInformationText ( ) << "\n";
  os << indent << "InformationCloseButton: " << this->GetInformationCloseButton ( ) << "\n";
  if ( this->DataTransfer )
    {
    this->GetDataTransfer()->PrintSelf(os, indent.GetNextIndent() );
    }
  }

//---------------------------------------------------------------------------
vtkSlicerDataTransferWidget::~vtkSlicerDataTransferWidget(void)
  {
    if ( this->TimerRunning )
      {
      this->KillTransferTimer();
      }
    this->TimerRunning = 0;
    this->TimerCount = 0;
    this->SetTimerID ( "" );
    this->SetParent ( NULL );
    this->SetCacheManager ( NULL );
    this->SetDataTransfer ( NULL );

    if ( this->InformationCloseButton )
      {
      this->InformationCloseButton->SetParent ( NULL );
      this->InformationCloseButton->Delete();
      this->InformationCloseButton = NULL;
      }
    if ( this->InformationText )
      {
      this->InformationText->SetParent ( NULL );
      this->InformationText->Delete();
      this->InformationText = NULL;
      }
    if ( this->InformationFrame )
      {
      this->InformationFrame->SetParent ( NULL );
      this->InformationFrame->Delete();
      this->InformationFrame = NULL;
      }
    if ( this->InformationTopLevel )
      {
      this->InformationTopLevel->SetParent ( NULL );
      this->InformationTopLevel->Delete();
      this->InformationTopLevel = NULL;
      }
    if ( this->InformationButton )
      {
      this->InformationButton->SetParent ( NULL );
      this->InformationButton->Delete();
      this->InformationButton = NULL;
      }
    if ( this->CancelButton )
      {
      this->CancelButton->SetParent ( NULL );
      this->CancelButton->Delete();
      this->CancelButton = NULL;
      }
    if ( this->DeleteButton )
      {
      this->DeleteButton->SetParent ( NULL );
      this->DeleteButton->Delete();
      this->DeleteButton = NULL;
      }
    if ( this->TransferStatusLabel )
      {
      this->TransferStatusLabel->SetParent ( NULL );
      this->TransferStatusLabel->Delete();
      this->TransferStatusLabel = NULL;
      }
    if ( this->TransferTypeLabel )
      {
      this->TransferTypeLabel->SetParent ( NULL );
      this->TransferTypeLabel->Delete();
      this->TransferTypeLabel = NULL;
      }
    if ( this->URILabel )
      {
      this->URILabel->SetParent ( NULL );
      this->URILabel->Delete();
      this->URILabel = NULL;
      }
    if ( this->DataTransferFrame )
      {
      this->DataTransferFrame->SetParent ( NULL );
      this->DataTransferFrame->Delete();
      this->DataTransferFrame = NULL;
      }
    if ( this->DataTransferIcons )
      {
      this->DataTransferIcons->Delete();
      this->DataTransferIcons = NULL;
      }
    this->SetApplication ( NULL );
  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::AddWidgetObservers ( )
  {


    this->CancelButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeleteButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationCloseButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    

  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::RemoveWidgetObservers( )
  {

    this->CancelButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeleteButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationCloseButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    

  }






//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {
    const char *msg;
    vtkKWMessageDialog *dialog;
    
    vtkDebugMacro ("vtkSlicerDataTransferWidget: Processing widget events.");
  // cancel data transfer
  if (this->CancelButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    if ( this->DataTransfer != NULL )
      {
      if ( this->DataTransfer->GetCancelRequested () == 0 )
        {
        this->DataTransfer->SetCancelRequested ( 1 );
        this->DataTransfer->SetTransferStatus ( vtkDataTransfer::CancelPending );
        }
      }
    }

  // display information about data transfer
  else if (this->InformationButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    // populate top level with information and display
    this->UpdateInformationText();
    this->DisplayInformationWindow();
    }

  // close the information top-level
  else if (this->InformationCloseButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    this->HideInformationWindow();
    }
  else if ( this->DeleteButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {

    if (( this->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::Running ))
      {
      // put up message dialog asking user to cancel the running transfer first, before deleteing from cache.
      dialog = vtkKWMessageDialog::New();
      dialog->SetParent ( this->GetParent() );
      dialog->SetStyleToMessage();
      msg = "Transfer is currently running. Either cancel the transfer first, or wait until it's finished to delete from cache.";
      dialog->SetText (msg);
      dialog->Create();
      dialog->Invoke();
      dialog->Delete();
      return;
      }
    else if (( this->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::CancelPending ))
      {
      // put up message dialog asking user to wait for the pending cancel to finish.
      dialog = vtkKWMessageDialog::New();
      dialog->SetParent ( this->GetParent() );
      dialog->SetStyleToMessage();
      msg = "A cancel is pending on this transfer. Please wait until the cancel is complete before deleting from cache.";
      dialog->SetText (msg);
      dialog->Create();
      dialog->Invoke();
      dialog->Delete();
      return;
      }
    else
      {
      // delete the particular data transfer from cache 
      this->DeleteTransferFromCache();
      // and disable the buttons.
      this->DisableCancelButton();
      this->DisableDeleteButton();
      this->UpdateURILabel ( "(cleared): ");
      this->DisableURILabel();
      }
    }
  }


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DisableDeleteButton ()
{
  this->DeleteButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheDisabledIcon() );
  this->DeleteButton->SetStateToDisabled();
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::EnableDeleteButton()
{
  this->DeleteButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheIcon() );
  this->DeleteButton->SetStateToNormal();
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DisableCancelButton ()
{
  this->CancelButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelDisabledIcon() );
  this->CancelButton->SetStateToDisabled();
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::EnableCancelButton()
{
  this->CancelButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelIcon() );
  this->CancelButton->SetStateToNormal();
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DisableURILabel()
{
  this->URILabel->SetForegroundColor ( 0.8, 0.8, 0.8);
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::EnableURILabel()
{
  this->URILabel->SetForegroundColor ( 0.4, 0.4, 0.4);
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateURILabel( const char *status)
{
  std::string str = status;
  if ( this->DataTransfer->GetTransferType() == vtkDataTransfer::RemoteDownload )
    {
    str += this->DataTransfer->GetSourceURI();
    }
  else if ( this->DataTransfer->GetTransferType() == vtkDataTransfer::RemoteUpload )
    {
    str += this->DataTransfer->GetDestinationURI();
    }
  this->URILabel->SetText ( str.c_str() );
}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DeleteTransferFromCache()
{
  if ( this->CacheManager != NULL )
    {
    if ( this->CacheManager->CachedFileExists( this->GetDataTransfer()->GetDestinationURI() ) )
      {
      if (( this->DataTransfer->GetTransferStatus() == vtkDataTransfer::Completed ) ||
          ( this->DataTransfer->GetTransferStatus() == vtkDataTransfer::CompletedWithErrors ) ||
          ( this->DataTransfer->GetTransferStatus() == vtkDataTransfer::Cancelled ) ||
          ( this->DataTransfer->GetTransferStatus() == vtkDataTransfer::TimedOut ))
        {
        this->CacheManager->DeleteFromCache ( this->GetDataTransfer()->GetDestinationURI() );
        this->UpdateURILabel ( "(cleared): ");
        this->DisableURILabel();
        this->DataTransfer->SetTransferStatus(vtkDataTransfer::Deleted );
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateWidget()
{
  vtkDebugMacro ( "vtkSlicerDataTransferWidget: UpdatingTransferWidget!!!" );


  //--- update the state of the widget to match its data transfer.
  if ( this->DataTransfer == NULL )
    {
    return;
    }

  switch ( this->DataTransfer->GetTransferType() )
    {
    case vtkDataTransfer::RemoteDownload:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeRemoteLoadIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: Remote download.");
      break;
    case vtkDataTransfer::RemoteUpload:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeRemoteSaveIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: Remote upload.");
      break;
    case vtkDataTransfer::LocalLoad:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeLoadIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: Load from local disk.");
      break;
    case vtkDataTransfer::LocalSave:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeSaveIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: Save to local disk.");
      break;
    case vtkDataTransfer::Unspecified:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeUnspecifiedIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: unspecified.");
      break;
    default:
      this->TransferTypeLabel->SetImageToIcon (this->DataTransferIcons->GetTransferTypeUnspecifiedIcon());
      this->TransferTypeLabel->SetBalloonHelpString ("Transfer type: unknown.");
      break;
    }

  //--- make sure Timer is running if the transfer is running, ready (loading from cache)
  // or cancel is pending... make sure Timer is killed if the transfer is done, or stopped.
  if ( (this->DataTransfer->GetTransferStatus() == vtkDataTransfer::Running ) ||
       (this->DataTransfer->GetTransferStatus() == vtkDataTransfer::CancelPending ) ||
       (this->DataTransfer->GetTransferStatus() == vtkDataTransfer::Ready ))
    {
    if ( !this->TimerRunning )
      {
      this->StartTransferTimer();
      }
    }
  else
    {
      if (  this->TimerRunning )
        {
        this->KillTransferTimer();
        }
    }
  
  switch ( this->DataTransfer->GetTransferStatus() )
    {
    case vtkDataTransfer::Idle:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusIdleIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: idle.");
      this->UpdateURILabel ( "(...): ");
      break;
    case vtkDataTransfer::Pending:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusIdleIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: idle.");
      this->UpdateURILabel ( "(...): ");
      break;
    case vtkDataTransfer::Running:
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: running.");
      this->UpdateURILabel ( "(running): ");
      break;
    case vtkDataTransfer::Completed:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusDoneIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: completed.");
      this->DisableCancelButton();
      //--- check to see if the transfer is still cached....
      //--- if present, label it as so. if not present,
      //--- label it as cleared.
      if ( this->DataTransfer->GetTransferType() == vtkDataTransfer::RemoteDownload )
        {
        if ( this->CacheManager->LocalFileExists ( this->DataTransfer->GetDestinationURI() ))
          {
          this->UpdateURILabel ( "(cached): ");
          }
        else
          {
          this->UpdateURILabel ( "(cleared): ");
          this->DisableURILabel();
          this->DisableDeleteButton();
          }
        }
      if ( this->DataTransfer->GetTransferType() == vtkDataTransfer::RemoteUpload )
        {
        if ( this->CacheManager->LocalFileExists ( this->DataTransfer->GetSourceURI() ))
          {
          this->UpdateURILabel ( "(cached): ");
          }
        else
          {
          this->UpdateURILabel ( "(cleared): ");
          this->DisableURILabel();
          this->DisableDeleteButton();
          }
        }
      break;
    case vtkDataTransfer::CompletedWithErrors:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusErrorIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: error!");
      this->DisableCancelButton();
      this->UpdateURILabel ( "(error): ");
      this->DisableURILabel();
      break;
    case vtkDataTransfer::CancelPending:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusCancelRequestedIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: cancel requested.");
      this->UpdateURILabel ( "(cancelling...): ");
      break;
    case vtkDataTransfer::Deleted:
      this->DisableDeleteButton();
      this->DisableCancelButton();
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: cleared from cache.");      
      this->UpdateURILabel ( "(cleared): ");
      this->DisableURILabel();
      break;
    case vtkDataTransfer::Cancelled:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusCancelledIcon());
      this->DisableCancelButton();
      this->DisableURILabel();
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: cancelled.");
      this->UpdateURILabel ( "(cancelled): ");
      this->DisableURILabel();
      break;
    case vtkDataTransfer::Ready:
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: loading from cache...");
      this->UpdateURILabel ( "(...): ");
      break;
    case vtkDataTransfer::TimedOut:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusTimedOutIcon());
      this->DisableCancelButton();
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: timed out.");
      this->UpdateURILabel ( "(timed out): ");
      this->DisableURILabel();
      break;
    default:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusIdleIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: unknown.");
      this->UpdateURILabel ( "(??): ");
      this->DisableURILabel();
      break;
    }
  this->UpdateInformationText();

}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateInformationText( )
{

  //--- update the state of the information text widget to match its data transfer.

  if ( this->DataTransfer == NULL )
    {
    return;
    }

  char *cmd = new char [1024];
  std::string infoString;

  vtkDebugMacro ("vtkSlicerDataTransferWidget: updating information text.");  
  this->InformationText->GetWidget()->QuickFormattingOn();

  infoString = "**Transfer type: **";
  infoString += this->DataTransfer->GetTransferTypeString ( );
  infoString += "\n";

  infoString += "**Transfer status: **";
  infoString += this->DataTransfer->GetTransferStatusString ( );
  infoString += "\n";

  infoString += "**Source URI: **";
  infoString += this->DataTransfer->GetSourceURI();
  infoString += "\n";  
  
  infoString += "**Destination URI: **";
  infoString += this->DataTransfer->GetDestinationURI();
  infoString += "\n";  
  
  infoString += "**Data transfer ID: **";
  sprintf ( cmd, "%d \n", this->DataTransfer->GetTransferID() );
  infoString += cmd;

  infoString += "**Destination MRMLNode ID: **";
  infoString += this->DataTransfer->GetTransferNodeID();
  infoString += "\n";  

  this->InformationText->GetWidget()->SetText( infoString.c_str());

  delete [] cmd;

}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DisplayInformationWindow( )
{
  if ( !this->InformationButton || !this->InformationButton->IsCreated() )
    {
    return;
    }

  vtkDebugMacro ("vtkSlicerDataTransferWidget: Displaying information window.");
  this->UpdateInformationText();

  //---Get the position of mouse, and InformationButton
  int x, y, py, ph;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  
  vtkKWTkUtilities::GetMousePointerCoordinates(this->InformationButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->InformationButton, NULL, &py);
  vtkKWTkUtilities::GetWidgetSize(this->InformationButton, NULL, &ph);
 
  // Place the window...
  x += ph;
  if (py <= y && y <= (py + ph -1))
    {
    y = py + ph - 3;
    }
  else
    {
    y -= ph;
    }
  this->InformationTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->InformationTopLevel->DeIconify();
  this->InformationTopLevel->Raise();
}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::HideInformationWindow( )
{
  if ( !this->InformationTopLevel )
    {
    return;
    }
  this->InformationTopLevel->Withdraw();
}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::CreateWidget( )
{

    //check if already created
    if (this->IsCreated())
      {
      vtkErrorMacro(<< this->GetClassName() << " already created");
      return;
      }
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
    if ( !app )
      {
      return;
      }
    if ( this->DataTransfer == NULL )
      {
      return;
      }

    vtkDebugMacro ("vtkSlicerDataTransferWidget: Creating widget.");
    //call the superclass to create the whole widget
    this->Superclass::CreateWidget();

    this->SetTimerID ("" );

    this->DataTransferFrame = vtkKWFrame::New();
    this->DataTransferFrame->SetParent ( this->GetParent () );
    this->DataTransferFrame->Create();
    this->Script ( "pack %s -side top -fill x -padx 1 -pady 1",
                   this->DataTransferFrame->GetWidgetName() );

    //---- Create some spacer labels to make things line up
    //--- nicely with the labels that head each column in the parent GUI.
    vtkKWLabel *l1 = vtkKWLabel::New();
    l1->SetParent ( this->DataTransferFrame );
    l1->Create();
    l1->SetWidth (6);
    l1->SetBorderWidth (0);
    l1->SetReliefToFlat();

    vtkKWLabel *l2 = vtkKWLabel::New();
    l2->SetParent ( this->DataTransferFrame );
    l2->Create();
    l2->SetWidth (6);
    l2->SetBorderWidth (0);
    l2->SetReliefToFlat();

    vtkKWLabel *l3 = vtkKWLabel::New();
    l3->SetParent ( this->DataTransferFrame );
    l3->Create();
    l3->SetWidth (6);
    l3->SetBorderWidth (0);
    l3->SetReliefToFlat();

    vtkKWLabel *l4 = vtkKWLabel::New();
    l4->SetParent ( this->DataTransferFrame );
    l4->Create();
    l4->SetWidth (6);
    l4->SetBorderWidth (0);
    l4->SetReliefToFlat();

    vtkKWLabel *l5 = vtkKWLabel::New();
    l5->SetParent ( this->DataTransferFrame );
    l5->Create();
    l5->SetWidth (6);
    l5->SetBorderWidth (0);
    l5->SetReliefToFlat();

    
    this->DataTransferIcons = vtkSlicerDataTransferIcons::New();
  
    this->URILabel = vtkKWLabel::New();
    this->URILabel->SetParent ( this->DataTransferFrame );
    this->URILabel->Create();
    this->URILabel->SetBorderWidth(0);
    this->UpdateURILabel ( "(starting...): ");
    this->EnableURILabel();

    this->TransferTypeLabel = vtkKWLabel::New();
    this->TransferTypeLabel->SetParent ( this->DataTransferFrame );
    this->TransferTypeLabel->Create();
    this->TransferTypeLabel->SetBorderWidth(0);
    switch ( this->DataTransfer->GetTransferType() )
      {
      case vtkDataTransfer::RemoteDownload:
        this->TransferTypeLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferTypeRemoteLoadIcon() );
        break;
      case vtkDataTransfer::RemoteUpload:
        this->TransferTypeLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferTypeRemoteSaveIcon() );
        break;
      case vtkDataTransfer::LocalLoad:
        this->TransferTypeLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferTypeLoadIcon() );
        break;
      case vtkDataTransfer::LocalSave:
        this->TransferTypeLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferTypeSaveIcon() );
        break;
      case vtkDataTransfer::Unspecified:
        this->TransferTypeLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferTypeUnspecifiedIcon() );
        break;
      default:
        break;
      }

    this->TransferStatusLabel = vtkKWLabel::New();
    this->TransferStatusLabel->SetParent ( this->DataTransferFrame );
    this->TransferStatusLabel->Create();
    this->TransferStatusLabel->SetBorderWidth(0);
    switch ( this->DataTransfer->GetTransferStatus() )
      {
      case vtkDataTransfer::Idle:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusIdleIcon() );
        break;
      case vtkDataTransfer::Ready:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusReadyIcon() );
        break;
      case vtkDataTransfer::Running:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusGoingIcon() );      
        break;
      case vtkDataTransfer::Completed:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusDoneIcon() );
        break;
      case vtkDataTransfer::CompletedWithErrors:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusErrorIcon() );
        break;
      case vtkDataTransfer::Cancelled:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusCancelledIcon() );
        break;
      case vtkDataTransfer::CancelPending:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusCancelRequestedIcon() );
        break;
      case vtkDataTransfer::TimedOut:
        this->TransferStatusLabel->SetImageToIcon ( this->DataTransferIcons->GetTransferStatusTimedOutIcon() );
        break;
      default:
        break;
      }

    this->CancelButton = vtkKWPushButton::New();
    this->CancelButton->SetParent ( this->DataTransferFrame);
    this->CancelButton->Create();
    this->CancelButton->SetReliefToFlat();
    this->CancelButton->SetBorderWidth(0);
    this->CancelButton->SetBalloonHelpString ( "Cancel this data transfer." );
    this->EnableCancelButton();
  
    this->DeleteButton = vtkKWPushButton::New();
    this->DeleteButton->SetParent ( this->DataTransferFrame);
    this->DeleteButton->Create();
    this->DeleteButton->SetReliefToFlat();
    this->DeleteButton->SetBorderWidth ( 0 );
    this->DeleteButton->SetBalloonHelpString ("Delete this data transfer from cache.");  
    this->EnableDeleteButton();
  
    this->InformationButton = vtkKWPushButton::New();
    this->InformationButton->SetParent ( this->DataTransferFrame );
    this->InformationButton->Create ( );
    this->InformationButton->SetReliefToFlat();
    this->InformationButton->SetBorderWidth(0);
    this->InformationButton->SetBalloonHelpString ( "View more detailed information about this data transfer." );
    this->InformationButton->SetImageToIcon ( this->DataTransferIcons->GetTransferInformationIcon() );

    this->InformationTopLevel = vtkKWTopLevel::New();
    this->InformationTopLevel->SetApplication (app );
    this->InformationTopLevel->SetMasterWindow ( this->GetParent() );
    this->InformationTopLevel->Create();
    this->InformationTopLevel->SetReliefToFlat();
    this->InformationTopLevel->SetBorderWidth(2);
    this->InformationTopLevel->SetDisplayPositionToPointer();
    this->InformationTopLevel->SetTitle ("Data transfer information");
    this->InformationTopLevel->SetSize ( 600, 150 );
    this->InformationTopLevel->Withdraw();
    this->InformationTopLevel->SetDeleteWindowProtocolCommand ( this, "HideInformationWindow" );

    this->InformationFrame = vtkKWFrame::New();
    this->InformationFrame->SetParent ( this->InformationTopLevel);
    this->InformationFrame->Create();

    this->InformationText = vtkKWTextWithScrollbars::New();
    this->InformationText->SetParent ( this->InformationFrame );
    this->InformationText->Create();
    this->InformationText->GetWidget()->SetText ( "No information available for this data transfer." );

    this->InformationCloseButton = vtkKWPushButton::New();
    this->InformationCloseButton->SetParent ( this->InformationFrame );
    this->InformationCloseButton->Create();
    this->InformationCloseButton->SetText ( "Close" );
    this->InformationCloseButton->SetWidth (10);
    this->InformationCloseButton->SetBalloonHelpString ("Dismiss this information window.");

    //--- pack up the information window.
    this->Script ( "pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
                   this->InformationFrame->GetWidgetName() );
    this->Script ( "pack %s -side bottom -anchor c -padx 2 -pady 2",
                   this->InformationCloseButton->GetWidgetName() );
    this->Script ( "pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
                   this->InformationText->GetWidgetName() );


    //--- grid everything up.

    this->Script ( "grid %s -row 0 -column 0 -sticky news -padx 2 -pady 2 ", l1->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -sticky news -padx 2 -pady 2 ", l2->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 2 -sticky news -padx 2 -pady 2 ", l3->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 3 -sticky news -padx 2 -pady 2 ", l4->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 4 -sticky news -padx 2 -pady 2 ", l5->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 5 -sticky nws -padx 4 -pady 2 ", this->URILabel->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 0 -weight 0", this->DataTransferFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 0", this->DataTransferFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 2 -weight 0", this->DataTransferFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 3 -weight 0", this->DataTransferFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 4 -weight 0", this->DataTransferFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 5 -weight 1", this->DataTransferFrame->GetWidgetName() );

    this->Script ( "place %s -in %s -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c -height 21 -bordermode inside",
                   this->TransferTypeLabel->GetWidgetName(), l1->GetWidgetName() );
    this->Script ( "place %s -in %s  -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c -height 21 -bordermode inside",
                   this->TransferStatusLabel->GetWidgetName(), l2->GetWidgetName() );
    this->Script ( "place %s -in %s  -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c -height 21 -bordermode inside",
                   this->CancelButton->GetWidgetName(), l3->GetWidgetName() );
    this->Script ( "place %s -in %s  -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c -height 21 -bordermode inside",
                   this->DeleteButton->GetWidgetName(), l4->GetWidgetName() );
    this->Script ( "place %s -in %s  -relx 0.5 -rely 0.5 -relheight 1.0 -anchor c -height 21 -bordermode inside",
                   this->InformationButton->GetWidgetName(), l5->GetWidgetName() );

    l1->Delete();
    l2->Delete();
    l3->Delete();
    l4->Delete();
    l5->Delete();
}




