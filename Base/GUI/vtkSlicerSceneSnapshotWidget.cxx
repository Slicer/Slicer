#include "vtkObject.h"
#include "vtkObjectFactory.h"

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
}


//---------------------------------------------------------------------------
vtkSlicerSceneSnapshotWidget::~vtkSlicerSceneSnapshotWidget ( )
{
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
   
  this->SetMRMLScene ( NULL );  
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
    this->RestoreSceneButton->SetBackgroundColor(1.0, 0.0, 0.0);
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
      //snapshotNode->Delete();
      }
    //this->RestoreSceneButton->SetBackgroundColor(0.0, 1.0, 0.0);

    }
  else if (this->RestoreSceneButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLSceneSnapshotNode *snapshotNode = 
      vtkMRMLSceneSnapshotNode::SafeDownCast(this->SnapshotSelectorWidget->GetSelected());
    if (snapshotNode)
      {
      this->MRMLScene->SaveStateForUndo();
      snapshotNode->RestoreScene();
      this->RestoreSceneButton->SetBackgroundColor(0.0, 1.0, 0.0);
      }
    }

} 



//---------------------------------------------------------------------------
void vtkSlicerSceneSnapshotWidget::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

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

  if (this->IsCreated())
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
  this->CreateSnapshotButton->SetText("Create Scene Snapshot");
  //this->CreateSnapshotButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->CreateSnapshotButton->GetWidgetName());

  // Restore FRAME            
  vtkKWFrame *frameRestore = vtkKWFrame::New ( );
  frameRestore->SetParent ( frame );
  frameRestore->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frameRestore->GetWidgetName() );  

  this->SnapshotSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->SnapshotSelectorWidget->SetParent ( frameRestore );
  this->SnapshotSelectorWidget->Create ( );
  this->SnapshotSelectorWidget->SetNodeClass("vtkMRMLSceneSnapshotNode", NULL, NULL, NULL);
  this->SnapshotSelectorWidget->SetShowHidden(1);
  this->SnapshotSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  //this->SnapshotSelectorWidget->SetBorderWidth(2);
  // this->SnapshotSelectorWidget->SetReliefToGroove();
  //this->SnapshotSelectorWidget->SetPadX(2);
  //this->SnapshotSelectorWidget->SetPadY(2);
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->SnapshotSelectorWidget->SetLabelText( "Scene Snapshot:");
  this->SnapshotSelectorWidget->SetBalloonHelpString("select a scene snapshot.");
  this->Script ( "pack %s -side left -anchor nw -fill x -padx 2 -pady 2",
                 this->SnapshotSelectorWidget->GetWidgetName());

  
  // Restore Snapshot button
  this->RestoreSceneButton = vtkKWPushButton::New();
  this->RestoreSceneButton->SetParent(frameRestore);
  this->RestoreSceneButton->Create();
  this->RestoreSceneButton->SetText("Restore");
  this->RestoreSceneButton->SetWidth ( 24);
  this->Script(
               "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
               this->RestoreSceneButton->GetWidgetName());
  
  if (this->MRMLScene != NULL)
    {
    this->SnapshotSelectorWidget->SetMRMLScene(this->MRMLScene);
    }

  this->CreateSnapshotButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RestoreSceneButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SnapshotSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  
  frame->Delete();
  frameRestore->Delete();
  
}
