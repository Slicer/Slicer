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
  this->SaveClipButton = NULL;
  this->ClipSelectorWidget = NULL;
  this->NameDialog = NULL;

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
  if (this->SaveClipButton)
    {
    this->SaveClipButton->SetParent(NULL);
    this->SaveClipButton->Delete();
    this->SaveClipButton = NULL;
    }
  if (this->ClipSelectorWidget)
    {
    this->ClipSelectorWidget->SetParent(NULL);
    this->ClipSelectorWidget->Delete();
    this->ClipSelectorWidget = NULL;
    }
  if (this->NameDialog)
    {
    this->NameDialog->SetParent(NULL);
    this->NameDialog->Delete();
    this->NameDialog = NULL;
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

  if (this->SaveClipButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->ClipSelectorWidget->SetSelectedNew("vtkMRMLSnapshotClipNode");
    this->ClipSelectorWidget->ProcessNewNodeCommand("vtkMRMLSnapshotClipNode", "SnapshotClip");
    vtkMRMLSnapshotClipNode *clipNode = 
      vtkMRMLSnapshotClipNode::SafeDownCast(this->ClipSelectorWidget->GetSelected());
    
    vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
    entry->GetWidget()->SetValue(clipNode->GetName());
    int result = this->NameDialog->Invoke();

    if (!result) 
      {
      this->MRMLScene->RemoveNode(clipNode);
      clipNode = NULL;
      }
    else 
      {
      clipNode->SetName(entry->GetWidget()->GetValue());
      this->ClipSelectorWidget->UpdateMenu();
      }
    if (clipNode)
      {
      for (unsigned int i=0; i<this->Sanpshots.size(); i++)
        {
        vtkMRMLSceneSnapshotNode *snapshotNode = this->Sanpshots[i];
        snapshotNode->SetSelectable(0);
        clipNode->AddSceneSanpshotNode(snapshotNode);
        this->MRMLScene->AddNodeNoNotify(snapshotNode);
        }
      this->Sanpshots.clear();
      }
    }
  else if (this->StartRecordButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->Sanpshots.clear();
    this->AddMRMLObservers();
    }

  else if (this->StopRecordButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->RemoveMRMLObservers();
    }


  else if (this->ReplayButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLSnapshotClipNode *clipNode = 
         vtkMRMLSnapshotClipNode::SafeDownCast(this->ClipSelectorWidget->GetSelected());
    if (this->Sanpshots.size() > 0 && clipNode == NULL)
      {
      this->MRMLScene->SaveStateForUndo();
      for (unsigned int i=0; i<this->Sanpshots.size(); i++)
        {
        vtkMRMLSceneSnapshotNode *snapshotNode = this->Sanpshots[i];
        snapshotNode->RestoreScene();
        // Cause Render
        //this->Script("sleep 2");
        this->Script("update");
        }
      }
    else if (clipNode != NULL)
      {
      this->MRMLScene->SaveStateForUndo();
      for ( int i=0; i< clipNode->GetNumberOfSceneSanpshotNodes(); i++)
        {
        vtkMRMLSceneSnapshotNode *snapshotNode = clipNode->GetSceneSanpshotNode(i);
        snapshotNode->RestoreScene();
        // Cause Render
        //this->Script("sleep 2");
        this->Script("update");
        }
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
  if (this->SaveClipButton)
    {
    this->SaveClipButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  
  this->NameDialog  = vtkKWSimpleEntryDialog::New();
  this->NameDialog->SetParent ( this->GetParent());
  this->NameDialog->SetTitle("Snapshot Clip Name");
  this->NameDialog->SetSize(400, 200);
  this->NameDialog->SetStyleToOkCancel();
  vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
  entry->SetLabelText("Clip Name");
  entry->GetWidget()->SetValue("");
  this->NameDialog->Create ( );


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

  // Create Snapshot button
  this->SaveClipButton = vtkKWPushButton::New();
  this->SaveClipButton->SetParent(frame);
  this->SaveClipButton->Create();
  this->SaveClipButton->SetText("Save Clip with Scene");
  //this->SaveClipButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->SaveClipButton->GetWidgetName());


  this->ClipSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->ClipSelectorWidget->SetParent ( frame);
  this->ClipSelectorWidget->Create ( );
  this->ClipSelectorWidget->SetNodeClass("vtkMRMLSnapshotClipNode", NULL, NULL, NULL);
  this->ClipSelectorWidget->SetShowHidden(1);
  this->ClipSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->ClipSelectorWidget->SetNoneEnabled(1);
  //this->ClipSelectorWidget->SetBorderWidth(2);
  // this->ClipSelectorWidget->SetReliefToGroove();
  //this->ClipSelectorWidget->SetPadX(2);
  //this->ClipSelectorWidget->SetPadY(2);
  this->ClipSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->ClipSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->ClipSelectorWidget->SetLabelText( "Snapshot Clip:");
  this->ClipSelectorWidget->SetBalloonHelpString("select a scene snapshot.");
  this->Script ( "pack %s -side left -anchor nw -fill x -padx 2 -pady 2",
                 this->ClipSelectorWidget->GetWidgetName());



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
    //this->AddMRMLObservers();
    }

  this->StartRecordButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StopRecordButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ReplayButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveClipButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  frame->Delete();
  
}

//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::AddMRMLObservers()
{
  int nnodes = this->MRMLScene->GetNumberOfNodes();
  vtkMRMLNode *node;
  for (int n=0; n<nnodes; n++)
    {    
    node = this->MRMLScene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode") &&
        !node->HasObserver(vtkCommand::AnyEvent, (vtkCommand *)this->MRMLCallbackCommand ))
      {
      node->AddObserver(vtkCommand::AnyEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerRecordSnapshotWidget::RemoveMRMLObservers()
{
  int nnodes = this->MRMLScene->GetNumberOfNodes();
  vtkMRMLNode *node;
  for (int n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNode(n);
    if (node && !node->IsA("vtkMRMLSceneSnapshotNode"))
      {
      node->RemoveObservers(vtkCommand::AnyEvent, (vtkCommand *)this->MRMLCallbackCommand );
      }
    }
}
