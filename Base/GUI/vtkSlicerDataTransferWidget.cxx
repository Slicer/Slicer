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
    this->RemoveWidgetObservers();
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
    if ( this->DataTransferIcons )
      {
      this->DataTransferIcons->Delete();
      this->DataTransferIcons = NULL;
      }
    if ( this->DataTransferFrame )
      {
      this->DataTransferFrame->SetParent ( NULL );
      this->DataTransferFrame->Delete();
      this->DataTransferFrame = NULL;
      }
    if ( this->DataTransfer )
      {
      vtkSetAndObserveMRMLNodeMacro ( this->DataTransfer, NULL);
      this->DataTransfer->Delete();
      }
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

  // cancel data transfer
  if (this->CancelButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    this->DataTransfer->SetCancelRequested ( 1 );
    this->UpdateWidget();
    }

  // display information about data transfer
  else if (this->InformationButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    // populate top level with information and display
    this->UpdateWidget();
    this->DisplayInformationWindow();
    }

  // close the information top-level
  else if (this->InformationCloseButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    this->HideInformationWindow();
    }
  else if ( this->DeleteButton == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent )
    {
    // delete the particular data transfer from cache and disable the buttons
    }
  }


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateWidget()
{
  //--- update the state of the widget to match its data transfer.
  this->URILabel->SetText ( this->DataTransfer->GetSourceURI() );
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
  switch ( this->DataTransfer->GetTransferStatus() )
    {
    case vtkDataTransfer::Idle:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusIdleIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: idle.");
      break;
    case vtkDataTransfer::Running:
      this->TransferStatusLabel->SetImageToIcon( this->DataTransferIcons->GetTransferStatusGoingIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: running.");
      break;
    case vtkDataTransfer::Completed:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusDoneIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: completed.");
      break;
    case vtkDataTransfer::CompletedWithErrors:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusErrorIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: error!");
      break;
    case vtkDataTransfer::CancelPending:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusCancelRequestedIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: cancel requested.");
      break;
    case vtkDataTransfer::Cancelled:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusCancelledIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: cancelled.");
      break;
    case vtkDataTransfer::Ready:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusReadyIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: ready.");
      break;
    case vtkDataTransfer::TimedOut:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusTimedOutIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: timed out.");
      break;
    default:
      this->TransferStatusLabel->SetImageToIcon(this->DataTransferIcons->GetTransferStatusIdleIcon());
      this->TransferStatusLabel->SetBalloonHelpString ("Transfer status: unknown.");
      break;
    }
  this->UpdateInformationText();

}


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateInformationText( )
{
  //--- update the state of the information text widget to match its data transfer.

  std::string infoString = "Data transfer ";
  char *command;

  command = new char [1024];
  sprintf ( command, "%d \n", this->DataTransfer->GetTransferID() );
  infoString += command;
  
  infoString += "Transfer type: ";
  infoString += this->DataTransfer->GetTransferTypeString ( );
  infoString += "\n";

  infoString += "Transfer status: ";
  infoString += this->DataTransfer->GetTransferStatusString ( );
  infoString += "\n";

  infoString += "Source URI: ";
  infoString += this->DataTransfer->GetSourceURI();
  infoString += "\n";  
  
  infoString += "Destination URI: ";
  infoString += this->DataTransfer->GetDestinationURI();
  infoString += "\n";  
  
  infoString += "Destination MRMLNode ID: ";
  infoString += this->DataTransfer->GetTransferNodeID();
  infoString += "\n";  

  this->InformationText->GetWidget()->SetText( infoString.c_str());

  delete [] command;
}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::DisplayInformationWindow( )
{
  if ( !this->InformationButton || !this->InformationButton->IsCreated() )
    {
    return;
    }

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
  if (this->IsCreated()){
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( !app )
    {
    return;
    }

  //call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  this->DataTransferFrame = vtkKWFrame::New();
  this->DataTransferFrame->SetParent ( this->GetParent () );
  this->DataTransferFrame->Create();
  this->Script ( "pack %s -side top, -anchor nw -fill x -padx 1 -pady 1",
                 this->DataTransferFrame->GetWidgetName() );

  this->DataTransferIcons = vtkSlicerDataTransferIcons::New();
  
  this->URILabel = vtkKWLabel::New();
  this->URILabel->SetParent ( this->DataTransferFrame );
  this->URILabel->Create();
  this->URILabel->SetText ( this->DataTransfer->GetSourceURI() );
  this->URILabel->SetBorderWidth(0);

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
  this->CancelButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelIcon() );
  
  this->DeleteButton = vtkKWPushButton::New();
  this->DeleteButton->SetParent ( this->DataTransferFrame);
  this->DeleteButton->Create();
  this->DeleteButton->SetReliefToFlat();
  this->DeleteButton->SetBorderWidth ( 0 );
  this->DeleteButton->SetBalloonHelpString ("Delete this data transfer from cache.");  
  this->DeleteButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheIcon() );
  
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
  this->InformationTopLevel->SetSize ( 200, 250 );
  this->InformationTopLevel->Withdraw();
  this->InformationTopLevel->SetDeleteWindowProtocolCommand ( this, "WithdrawHelpWindow" );

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
  this->InformationCloseButton->SetReliefToFlat();
  this->InformationCloseButton->SetBorderWidth(0);
  this->InformationCloseButton->SetBalloonHelpString ("Dismiss this information window.");

  //--- pack everything up.
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->TransferTypeLabel->GetWidgetName() );
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->URILabel->GetWidgetName() );
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->TransferStatusLabel->GetWidgetName() );
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->CancelButton->GetWidgetName() );
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->DeleteButton->GetWidgetName() );
  this->Script ( "pack %s -side left, -anchor nw -fill y -padx 2 -pady 2",
                 this->InformationButton->GetWidgetName() );

  //--- pack up the information window.
  this->Script ( "pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
                 this->InformationText->GetWidgetName() );
  this->Script ( "pack %s -side bottom -anchor c -padx 2 -pady 2",
                 this->InformationCloseButton->GetWidgetName() );

  }

