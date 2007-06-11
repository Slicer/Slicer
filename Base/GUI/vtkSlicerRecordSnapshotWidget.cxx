#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerRecordSnapshotWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWSimpleEntryDialog.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

// to get at the colour logic to set a default color node
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerColorGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerRecordSnapshotWidget );
vtkCxxRevisionMacro ( vtkSlicerRecordSnapshotWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerRecordSnapshotWidget::vtkSlicerRecordSnapshotWidget ( )
{

  this->StartRecordButton = NULL;
  this->StopRecordButton = NULL;
  this->ReplayButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerRecordSnapshotWidget::~vtkSlicerRecordSnapshotWidget ( )
{
  if (this->ReplayButton)
    {
    this->ReplayButton->SetParent(NULL);
    this->ReplayButton->Delete();
    this->ReplayButton = NULL;
    }
  if (this->StopRecordButton)
    {
    this->StopRecordButton->SetParent(NULL);
    this->StopRecordButton->Delete();
    this->StopRecordButton = NULL;
    }
  if (this->StartRecordButton)
    {
    this->StartRecordButton->SetParent(NULL);
    this->StartRecordButton->Delete();
    this->StartRecordButton = NULL;
    }
   
  this->SetMRMLScene ( NULL );  
}


//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerRecordSnapshotWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  //
  // process volume selector events
  //
  if (this->StartRecordButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->Sanpshots.clear();
    this->AddMRMLObseervers();
    }

  else if (this->StopRecordButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->RemoveMRMLObseervers();
    }


  else if (this->ReplayButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    for (unsigned int i=0; i<this->Sanpshots.size(); i++)
      {
      vtkMRMLSceneSnapshotNode *snapshotNode = this->Sanpshots[i];
      snapshotNode->RestoreScene();
      // Cause Render
      //this->Script("sleep 2");
      this->Script("update");
      }
    }

} 



//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  if (vtkMRMLNode::SafeDownCast(caller)  && event == vtkCommand::ModifiedEvent )
    {
    vtkMRMLSceneSnapshotNode *snapshotNode = vtkMRMLSceneSnapshotNode::New();
    snapshotNode->SetScene(this->MRMLScene);
    snapshotNode->StoreScene();
    Sanpshots.push_back(snapshotNode);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::RemoveWidgetObservers ( ) {
  if ( this->StartRecordButton)
    {
    this->StartRecordButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->StopRecordButton)
    {
    this->StopRecordButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ReplayButton)
    {
    this->ReplayButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
  

  // FRAME            
  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );


  // Create Start Record button
  this->StartRecordButton = vtkKWPushButton::New();
  this->StartRecordButton->SetParent(frame);
  this->StartRecordButton->Create();
  this->StartRecordButton->SetText("Start Recording");
  //this->StartRecordButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->StartRecordButton->GetWidgetName());

  // Create Stop Record button
  this->StopRecordButton = vtkKWPushButton::New();
  this->StopRecordButton->SetParent(frame);
  this->StopRecordButton->Create();
  this->StopRecordButton->SetText("Stop Recording");
  //this->StopRecordButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->StopRecordButton->GetWidgetName());

  // Restore Snapshot button
  this->ReplayButton = vtkKWPushButton::New();
  this->ReplayButton->SetParent(frame);
  this->ReplayButton->Create();
  this->ReplayButton->SetText("Replay");
  this->ReplayButton->SetWidth ( 24);
  this->Script(
               "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->ReplayButton->GetWidgetName());
  
  if (this->MRMLScene != NULL)
    {
    //this->AddMRMLObseervers();
    }

  this->StartRecordButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StopRecordButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ReplayButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  
  frame->Delete();
  
}

//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::AddMRMLObseervers()
{
  int nnodes = this->MRMLScene->GetNumberOfNodes();
  vtkMRMLNode *node;
  for (int n=0; n<nnodes; n++)
    {    
    node = this->MRMLScene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode"))
      {
      node->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::RemoveMRMLObseervers()
{
  int nnodes = this->MRMLScene->GetNumberOfNodes();
  vtkMRMLNode *node;
  for (int n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode"))
      {
      node->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}
