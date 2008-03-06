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
#include "vtkSlicerDataTransferIcons.h"


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
    this->InformationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationCloseButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::RemoveWidgetObservers( )
  {
    this->CancelButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->InformationCloseButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  }


//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateInfo()
  {
    // update gui values
  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::UpdateWidget()
  {
    // get any new state information
    // then update gui
    this->UpdateInfo(); 
  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::PopulateInformationDisplay ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::InvokeTransferCancelEvent ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::ProcessWidgetEvents (vtkObject *caller, unsigned long event, void *callData)
  {

  // cancel data transfer
  if (this->CancelButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    this->DataTransfer->SetCancelRequested ( 1 );
    this->InvokeTransferCancelEvent ( );
    
    this->UpdateWidget();
    }

  // display information about data transfer
  else if (this->InformationButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    // populate top level with information and display
    this->PopulateInformationDisplay ( );
    this->UpdateWidget();
    }

  // close the information top-level
  else if (this->InformationCloseButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent)
    {
    }

  }

//---------------------------------------------------------------------------
void vtkSlicerDataTransferWidget::CreateWidget( )
  {
  //check if already created
  if (this->IsCreated()){
    vtkErrorMacro(<< this->GetClassName() << " already created");
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
  this->URILabel->SetText ( "" );

  this->TransferTypeLabel = vtkKWLabel::New();
  this->TransferTypeLabel->SetParent ( this->DataTransferFrame );
  this->TransferTypeLabel->Create();

  this->TransferStatusLabel = vtkKWLabel::New();
  this->TransferStatusLabel->SetParent ( this->DataTransferFrame );
  this->TransferStatusLabel->Create();

  this->CancelButton = vtkKWPushButton::New();
  this->CancelButton->SetParent ( this->DataTransferFrame);
  this->CancelButton->Create();
  this->CancelButton->SetText ( "Cancel" );
  
  this->InformationButton = vtkKWPushButton::New();
  this->InformationButton->SetParent ( this->DataTransferFrame );
  this->InformationButton->Create ( );
  this->InformationButton->SetText ( "Information" );

  this->InformationTopLevel = vtkKWTopLevel::New();
  this->InformationTopLevel->SetParent ( this->DataTransferFrame );
  this->InformationTopLevel->Create();

  this->InformationFrame = vtkKWFrame::New();
  this->InformationFrame->SetParent ( this->InformationTopLevel);
  this->InformationFrame->Create();

  this->InformationText = vtkKWTextWithScrollbars::New();
  this->InformationText->SetParent ( this->InformationFrame );
  this->InformationText->Create();

  this->InformationCloseButton = vtkKWPushButton::New();
  this->InformationCloseButton->SetParent ( this->InformationFrame );
  this->InformationCloseButton->Create();
  this->InformationCloseButton->SetText ( "Close" );

  }
