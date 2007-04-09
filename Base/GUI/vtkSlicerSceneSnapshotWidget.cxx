#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerSceneSnapshotWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

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
}


//---------------------------------------------------------------------------
vtkSlicerSceneSnapshotWidget::~vtkSlicerSceneSnapshotWidget ( )
{
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
  if (this->CreateSnapshotButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    this->SnapshotSelectorWidget->SetSelectedNew("vtkMRMLSceneSnapshotNode");
    this->SnapshotSelectorWidget->ProcessNewNodeCommand("vtkMRMLSceneSnapshotNode", "SceneSnapshot");
    vtkMRMLSceneSnapshotNode *snapshotNode = 
      vtkMRMLSceneSnapshotNode::SafeDownCast(this->SnapshotSelectorWidget->GetSelected());
    if (snapshotNode)
      {
      snapshotNode->StoreScene();
      }
    }
  else if (this->RestoreSceneButton == vtkKWPushButton::SafeDownCast(caller)  && event == vtkKWPushButton::InvokedEvent )
    {
    vtkMRMLSceneSnapshotNode *snapshotNode = 
      vtkMRMLSceneSnapshotNode::SafeDownCast(this->SnapshotSelectorWidget->GetSelected());
    if (snapshotNode)
      {
      snapshotNode->RestoreScene();
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
  this->SnapshotSelectorWidget->SetMRMLScene(NULL);
  this->SnapshotSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CreateSnapshotButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RestoreSceneButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
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
  
  // ---
  // DISPLAY FRAME            
  vtkKWFrame *frame = vtkKWFrame::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );
  
  this->SnapshotSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->SnapshotSelectorWidget->SetParent ( frame );
  this->SnapshotSelectorWidget->Create ( );
  this->SnapshotSelectorWidget->SetNodeClass("vtkMRMLSceneSnapshotNode", NULL, NULL, NULL);
  this->SnapshotSelectorWidget->SetShowHidden(1);
  this->SnapshotSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->SnapshotSelectorWidget->SetBorderWidth(2);
  // this->SnapshotSelectorWidget->SetReliefToGroove();
  this->SnapshotSelectorWidget->SetPadX(2);
  this->SnapshotSelectorWidget->SetPadY(2);
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->SnapshotSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->SnapshotSelectorWidget->SetLabelText( "Scene Snapshot Select: ");
  this->SnapshotSelectorWidget->SetBalloonHelpString("select a scene snapshot.");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 this->SnapshotSelectorWidget->GetWidgetName());
  this->SnapshotSelectorWidget->SetWidgetName("SnapshotSelector");

    // CreateSnapshot button
  this->CreateSnapshotButton = vtkKWPushButton::New();
  this->CreateSnapshotButton->SetParent(frame);
  this->CreateSnapshotButton->Create();
  this->CreateSnapshotButton->SetText("Create Scene Snapshot");
  this->CreateSnapshotButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->CreateSnapshotButton->GetWidgetName());
  
  
  // CreateSnapshot button
  this->RestoreSceneButton = vtkKWPushButton::New();
  this->RestoreSceneButton->SetParent(frame);
  this->RestoreSceneButton->Create();
  this->RestoreSceneButton->SetText("Restore Scene from Snapshot");
  this->RestoreSceneButton->SetWidth ( 40);
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->RestoreSceneButton->GetWidgetName());
  

  if (this->MRMLScene != NULL)
    {
    this->SnapshotSelectorWidget->SetMRMLScene(this->MRMLScene);
    }

  this->SnapshotSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CreateSnapshotButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RestoreSceneButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  
  frame->Delete();
  
}
