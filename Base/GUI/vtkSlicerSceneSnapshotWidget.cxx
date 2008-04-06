#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerSceneSnapshotWidget.h"

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
#include "vtkSlicerFoundationIcons.h"
#include "vtkMRMLScalarVolumeNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSceneSnapshotWidget );
vtkCxxRevisionMacro ( vtkSlicerSceneSnapshotWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSceneSnapshotWidget::vtkSlicerSceneSnapshotWidget ( )
{

    this->SnapshotSelectorWidget = NULL;
    this->RestoreSceneButton = NULL;
    this->CreateSnapshotButton = NULL;
    this->NameDialog = NULL;
    this->Icons = vtkSlicerFoundationIcons::New();

    //--- set up callback
    this->MRMLExtraCallbackCommand = vtkCallbackCommand::New ( );
    this->MRMLExtraCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->MRMLExtraCallbackCommand->SetCallback( this->MRMLExtraCallback );
}


//---------------------------------------------------------------------------
vtkSlicerSceneSnapshotWidget::~vtkSlicerSceneSnapshotWidget ( )
{
  if ( this->Icons )
    {
    this->Icons->Delete();
    this->Icons = NULL;
    }
  if (this->SnapshotSelectorWidget)
    {
    this->NameDialog->SetParent(NULL);
    this->NameDialog->Delete();
    this->NameDialog = NULL;
    }
  if (this->SnapshotSelectorWidget)
    {
    this->SnapshotSelectorWidget->SetParent(NULL);
    this->SnapshotSelectorWidget->Delete();
    this->SnapshotSelectorWidget = NULL;
    }
  if (this->RestoreSceneButton)
    {
    this->RestoreSceneButton->SetParent(NULL);
    this->RestoreSceneButton->Delete();
    this->RestoreSceneButton = NULL;
    }
  if (this->CreateSnapshotButton)
    {
    this->CreateSnapshotButton->SetParent(NULL);
    this->CreateSnapshotButton->Delete();
    this->CreateSnapshotButton = NULL;
    }
  if ( this->MRMLExtraCallbackCommand )
    {
    this->MRMLExtraCallbackCommand->Delete();
    this->MRMLExtraCallbackCommand = NULL;
    }
  this->SetMRMLScene ( NULL );
  this->SetApplication ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerSceneSnapshotWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                         unsigned long event, void *callData )
{

  //
  // process volume selector events
  //
  if (this->SnapshotSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && event ==  vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    this->RestoreSceneButton->SetImageToIcon ( this->Icons->GetSlicerGoIcon() );
    this->RestoreSceneButton->SetBalloonHelpString ("Restore a scene snapshot." );
    this->RestoreSceneButton->SetStateToNormal();
    }
  else if (this->CreateSnapshotButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->SnapshotSelectorWidget->SetSelectedNew("vtkMRMLSceneSnapshotNode");
    this->SnapshotSelectorWidget->ProcessNewNodeCommand("vtkMRMLSceneSnapshotNode", "SceneSnapshot");
    vtkMRMLSceneSnapshotNode *snapshotNode = 
      vtkMRMLSceneSnapshotNode::SafeDownCast(this->SnapshotSelectorWidget->GetSelected());
      
    vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
    entry->GetWidget()->SetValue(snapshotNode->GetName());
    int result = this->NameDialog->Invoke();

    if (!result) 
      {
      this->MRMLScene->RemoveNode(snapshotNode);
      snapshotNode = NULL;
      }
    else 
      {
      snapshotNode->SetName(entry->GetWidget()->GetValue());
      this->SnapshotSelectorWidget->UpdateMenu();
      }
    if (snapshotNode)
      {
      snapshotNode->StoreScene();
      this->RestoreSceneButton->SetImageToIcon ( this->Icons->GetSlicerGoIcon() );
      this->RestoreSceneButton->SetBalloonHelpString ("Restore a scene snapshot." );
      this->RestoreSceneButton->SetStateToNormal();
      //snapshotNode->Delete();
      }

    }
  else if (this->RestoreSceneButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLSceneSnapshotNode *snapshotNode = 
      vtkMRMLSceneSnapshotNode::SafeDownCast(this->SnapshotSelectorWidget->GetSelected());
    if (snapshotNode)
      {
      this->MRMLScene->SaveStateForUndo();
      snapshotNode->RestoreScene();
//      this->RestoreSceneButton->SetImageToIcon ( this->Icons->GetSlicerDoneIcon() );
      }
    }
} 



//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::MRMLExtraCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{

  static int inMRMLExtraCallback = 0;

  if ( inMRMLExtraCallback )
    {
    vtkDebugWithObjectMacro (caller, << "*********MRMLCallback called recursively?" << endl);
    return;
    }
  inMRMLExtraCallback = 1;

  vtkSlicerSceneSnapshotWidget *self = reinterpret_cast <vtkSlicerSceneSnapshotWidget *> (clientData );
  
  vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
  if ( self->GetMRMLScene() == NULL )
    {
    vtkErrorWithObjectMacro ( self, << "*********MRMLCallback called recursively?" << endl);
    return;
    }

  if (scene == self->GetMRMLScene() )
    {
    if  (eid == vtkMRMLScene::SceneCloseEvent )
      {
        self->RestoreSceneButton->SetImageToIcon (self->Icons->GetSlicerGoIcon() );
        self->RestoreSceneButton->SetBalloonHelpString ("Restore a scene snapshot (no snapshots currently available)." );
        self->RestoreSceneButton->SetStateToDisabled();
      }
    else if  (eid == vtkMRMLScene::NodeRemovedEvent )
      {
      //--- count number of vtkMRMLSnapshotNodes in the scene.
      //--- if there are none, then make the GUI current
    
      int num = self->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSnapshotNode");
      if ( num <= 0 )
        {
        self->RestoreSceneButton->SetImageToIcon (self->Icons->GetSlicerGoIcon() );
        self->RestoreSceneButton->SetBalloonHelpString ("Restore a scene snapshot (no snapshots currently available)." );
        self->RestoreSceneButton->SetStateToDisabled();
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::AddMRMLObservers ( )
{
  if (this->MRMLScene == NULL)
    {
    return;
    }
  if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLExtraCallbackCommand) != 1)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLExtraCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLExtraCallbackCommand);
    }
  else
    {
    vtkDebugMacro("MRML scene already has the node removed event being watched by the SceneSnapshotWidget");
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::RemoveMRMLObservers ()
{
  if ( this->MRMLScene == NULL )
    {
    return;
    }
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLExtraCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLExtraCallbackCommand);
}



//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

  // Looking for node deleted event...
  // Check to see if there are any vtkMRMLSceneSnapshotNodes in
  // the scene. If there are none, disable the display snapshot button.



}



//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::RemoveWidgetObservers ( ) {
  if ( this->CreateSnapshotButton)
    {
    this->CreateSnapshotButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->RestoreSceneButton)
    {
    this->RestoreSceneButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SnapshotSelectorWidget)
    {
    this->SnapshotSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated() )
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();
  
  this->NameDialog  = vtkKWSimpleEntryDialog::New();
  this->NameDialog->SetParent ( this->GetParent());
  this->NameDialog->SetTitle("Scene Snapshot Name");
  this->NameDialog->SetSize(400, 200);
  this->NameDialog->SetStyleToOkCancel();
  vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
  entry->SetLabelText("Snapshot Name");
  entry->GetWidget()->SetValue("");
  this->NameDialog->Create ( );

  // FRAME            
  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // Create Snapshot button
  this->CreateSnapshotButton = vtkKWPushButton::New();
  this->CreateSnapshotButton->SetParent(frame);
  this->CreateSnapshotButton->Create();
  this->CreateSnapshotButton->SetImageToIcon (this->Icons->GetSlicerCameraIcon() );
  this->CreateSnapshotButton->SetReliefToFlat();
  this->CreateSnapshotButton->SetBorderWidth(0);
  this->CreateSnapshotButton->SetBalloonHelpString ( "Capture a scene snapshot." );
//  this->CreateSnapshotButton->SetText("Create Scene Snapshot");
  //this->CreateSnapshotButton->SetWidth ( 40);
  this->Script( "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->CreateSnapshotButton->GetWidgetName());


  this->SnapshotSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->SnapshotSelectorWidget->SetParent ( frame );
  this->SnapshotSelectorWidget->Create ( );
  this->SnapshotSelectorWidget->SetNodeClass("vtkMRMLSceneSnapshotNode", NULL, NULL, NULL);
  this->SnapshotSelectorWidget->SetShowHidden(1);
  this->SnapshotSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  //this->SnapshotSelectorWidget->SetBorderWidth(2);
  // this->SnapshotSelectorWidget->SetReliefToGroove();
  //this->SnapshotSelectorWidget->SetPadX(2);
  //this->SnapshotSelectorWidget->SetPadY(2);
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->SetWidth(30);
  this->SnapshotSelectorWidget->SetLabelText( "");
  this->SnapshotSelectorWidget->SetBalloonHelpString("select a scene snapshot.");
  this->Script ( "pack %s -side left -anchor nw -padx 2 -pady 2",
                 this->SnapshotSelectorWidget->GetWidgetName());
  
  // Restore Snapshot button
  this->RestoreSceneButton = vtkKWPushButton::New();
  this->RestoreSceneButton->SetParent(frame);
  this->RestoreSceneButton->Create();
  this->RestoreSceneButton->SetImageToIcon (this->Icons->GetSlicerGoIcon() );
  this->RestoreSceneButton->SetReliefToFlat();
  this->RestoreSceneButton->SetBorderWidth(0);
  this->RestoreSceneButton->SetStateToDisabled();
  this->RestoreSceneButton->SetBalloonHelpString ("Restore a scene snapshot." );
//  this->RestoreSceneButton->SetText("Restore");
//  this->RestoreSceneButton->SetWidth ( 24);
  this->Script("pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->RestoreSceneButton->GetWidgetName());
  
  if (this->MRMLScene != NULL)
    {
    this->SnapshotSelectorWidget->SetMRMLScene(this->MRMLScene);
    }

  this->CreateSnapshotButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RestoreSceneButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SnapshotSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  frame->Delete();

}

