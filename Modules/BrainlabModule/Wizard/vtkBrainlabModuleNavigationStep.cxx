#include "vtkBrainlabModuleNavigationStep.h"

#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMatrix4x4.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkKWInternationalization.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkMRMLSliceCompositeNode.h"

 
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleNavigationStep);
vtkCxxRevisionMacro(vtkBrainlabModuleNavigationStep, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkBrainlabModuleNavigationStep::vtkBrainlabModuleNavigationStep()
{
  this->SetName("3/3. Navigate");
  this->SetDescription("Navigate with DTI visualization.");

  this->FiducialSeedingButton = NULL; 
  this->FiducialSeedingFrame = NULL;

  this->MRMLTreeButton = NULL; 
  this->MRMLTreeFrame = NULL;

  this->SliceControlFrame = NULL;
  this->ButtonFrame = NULL;
  this->AxialCheckButton = NULL;
  this->SagittalCheckButton = NULL;
  this->CoronalCheckButton = NULL;
  this->TrackingSourceSelectorWidget = NULL;

  this->TimerFlag = 0;
  this->TimerInterval = 100; // 100 ms

  this->SliceNode[0] = NULL;
  this->SliceNode[1] = NULL;
  this->SliceNode[2] = NULL;
  this->EnableOblique = false;

}



//----------------------------------------------------------------------------
vtkBrainlabModuleNavigationStep::~vtkBrainlabModuleNavigationStep()
{  
  if(this->FiducialSeedingButton)
    {
    this->FiducialSeedingButton->Delete();
    this->FiducialSeedingButton = NULL;
    }

  if(this->FiducialSeedingFrame)
    {
    this->FiducialSeedingFrame->Delete();
    this->FiducialSeedingFrame = NULL;
    }

  if(this->MRMLTreeButton)
    {
    this->MRMLTreeButton->Delete();
    this->MRMLTreeButton = NULL;
    }

  if(this->MRMLTreeFrame)
    {
    this->MRMLTreeFrame->Delete();
    this->MRMLTreeFrame = NULL;
    }

  if(this->SliceControlFrame)
    {
    this->SliceControlFrame->Delete();
    this->SliceControlFrame = NULL;
    }
  if(this->ButtonFrame)
    {
    this->ButtonFrame->Delete();
    this->ButtonFrame = NULL;
    }
   if(this->AxialCheckButton)
    {
    this->AxialCheckButton->Delete();
    this->AxialCheckButton = NULL;
    }
   if(this->SagittalCheckButton)
    {
    this->SagittalCheckButton->Delete();
    this->SagittalCheckButton = NULL;
    }
   if(this->CoronalCheckButton)
    {
    this->CoronalCheckButton->Delete();
    this->CoronalCheckButton = NULL;
    }
   if(this->TrackingSourceSelectorWidget)
    {
    this->TrackingSourceSelectorWidget->Delete();
    this->TrackingSourceSelectorWidget = NULL;
    }

}



//----------------------------------------------------------------------------
void vtkBrainlabModuleNavigationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);
//  vtkKWWidget *parent = wizard_widget->GetClientArea();

  //---
  // Slice control frame
  // -------------------------------------------------------------------------------------------------
  if (! this->SliceControlFrame)
    {
    this->SliceControlFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SliceControlFrame->IsCreated())
    { 
    this->SliceControlFrame->SetParent(wizard_widget->GetClientArea());
    this->SliceControlFrame->Create();
    this->SliceControlFrame->SetLabelText("Slice Control"); 
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->SliceControlFrame->GetWidgetName());
  
  if (! this->TrackingSourceSelectorWidget)
    {
    this->TrackingSourceSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->TrackingSourceSelectorWidget->SetParent(this->SliceControlFrame->GetFrame());
    this->TrackingSourceSelectorWidget->Create();
    this->TrackingSourceSelectorWidget->SetNodeClass("vtkMRMLLinearTransformNode", NULL,
                                                  NULL, "LinearTransform");
    this->TrackingSourceSelectorWidget->SetMRMLScene(this->GetGUI()->GetMRMLManager()->GetMRMLScene());
    this->TrackingSourceSelectorWidget->SetBorderWidth(2);
    this->TrackingSourceSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOn();
    this->TrackingSourceSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->TrackingSourceSelectorWidget->SetNoneEnabled(1);
    this->TrackingSourceSelectorWidget->SetNewNodeEnabled(0);
    this->TrackingSourceSelectorWidget->SetLabelText( "Tracking source: ");
    this->TrackingSourceSelectorWidget->SetBalloonHelpString("Select a transform from the current scene.");
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->TrackingSourceSelectorWidget->GetWidgetName());


  if (! this->ButtonFrame)
    {
    this->ButtonFrame = vtkKWFrame::New();
    }
  if (!this->ButtonFrame->IsCreated())
    { 
    this->ButtonFrame->SetParent(this->SliceControlFrame->GetFrame());
    this->ButtonFrame->Create();
    }
  this->Script("pack %s -side top -anchor center -expand n -fill both -padx 0 -pady 4", 
               this->ButtonFrame->GetWidgetName());

  if (! this->AxialCheckButton)
    {
    this->AxialCheckButton = vtkKWCheckButton::New();
    this->AxialCheckButton->SetParent(this->ButtonFrame);
    this->AxialCheckButton->Create();
    this->AxialCheckButton->SelectedStateOn();
    this->AxialCheckButton->SetText("Axial");
    }
   if (! this->SagittalCheckButton)
    {
    this->SagittalCheckButton = vtkKWCheckButton::New();
    this->SagittalCheckButton->SetParent(this->ButtonFrame);
    this->SagittalCheckButton->Create();
    this->SagittalCheckButton->SelectedStateOn();
    this->SagittalCheckButton->SetText("Sagittal");
    }
  if (! this->CoronalCheckButton)
    {
    this->CoronalCheckButton = vtkKWCheckButton::New();
    this->CoronalCheckButton->SetParent(this->ButtonFrame);
    this->CoronalCheckButton->Create();
    this->CoronalCheckButton->SelectedStateOn();
    this->CoronalCheckButton->SetText("Coronal");
    }
  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->AxialCheckButton->GetWidgetName(),
               this->SagittalCheckButton->GetWidgetName(),
               this->CoronalCheckButton->GetWidgetName());

  //---
  // Fiducial seeding frame
  // -------------------------------------------------------------------------------------------------
  if (! this->FiducialSeedingFrame)
    {
    this->FiducialSeedingFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->FiducialSeedingFrame->IsCreated())
    { 
    this->FiducialSeedingFrame->SetParent(wizard_widget->GetClientArea());
    this->FiducialSeedingFrame->Create();
    this->FiducialSeedingFrame->SetLabelText("Fiducial Seeding"); 
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->FiducialSeedingFrame->GetWidgetName());

  if (!this->FiducialSeedingButton)
    {
    this->FiducialSeedingButton = vtkKWPushButton::New();
    this->FiducialSeedingButton->SetParent (this->FiducialSeedingFrame->GetFrame());
    this->FiducialSeedingButton->Create();
    this->FiducialSeedingButton->SetText("FiducialSeeding Module");
    this->FiducialSeedingButton->SetCommand(this, "FiducialSeedingButtonCallback");
    this->FiducialSeedingButton->SetWidth(25);
    }
  this->Script("pack %s -side top -anchor center -padx 2 -pady 5", this->FiducialSeedingButton->GetWidgetName());

  //---
  // MRML tree frame
  // -------------------------------------------------------------------------------------------------
  if (! this->MRMLTreeFrame)
    {
    this->MRMLTreeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->MRMLTreeFrame->IsCreated())
    { 
    this->MRMLTreeFrame->SetParent(wizard_widget->GetClientArea());
    this->MRMLTreeFrame->Create();
    this->MRMLTreeFrame->SetLabelText("MRML Tree"); 
    }
  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 4", 
               this->MRMLTreeFrame->GetWidgetName());

  if (!this->MRMLTreeButton)
    {
    this->MRMLTreeButton = vtkKWPushButton::New();
    this->MRMLTreeButton->SetParent (this->MRMLTreeFrame->GetFrame());
    this->MRMLTreeButton->Create();
    this->MRMLTreeButton->SetText("Data Module");
    this->MRMLTreeButton->SetCommand(this, "MRMLTreeButtonCallback");
    this->MRMLTreeButton->SetWidth(25);
    }
  this->Script(
    "pack %s -side top -anchor center -padx 2 -pady 5", 
    this->MRMLTreeButton->GetWidgetName());

   //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Navigate Step");
  msg_dlg1->SetText("This is the third step in BrainlabModule. In this step, the user navigates with DTI visualization."
                   );

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}
 


//----------------------------------------------------------------------------
void vtkBrainlabModuleNavigationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkBrainlabModuleNavigationStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{


}



void vtkBrainlabModuleNavigationStep::FiducialSeedingButtonCallback()
{
  // FiducialSeedingButton Pressed

  if (this->FiducialSeedingButton)
    {
    this->RaiseModule("FiducialSeeding");
    }
}



void vtkBrainlabModuleNavigationStep::MRMLTreeButtonCallback()
{
  // MRMLTreeButton Pressed

  if (this->MRMLTreeButton)
    {
    this->RaiseModule("Data");
    }
}



void vtkBrainlabModuleNavigationStep::ProcessTimerEvents()
{

  if (this->TimerFlag)
    {
    if (this->TrackingSourceSelectorWidget)
      {
      vtkMRMLLinearTransformNode* transNode =    
        vtkMRMLLinearTransformNode::SafeDownCast(this->TrackingSourceSelectorWidget->GetSelected());
      int vis = (transNode ? 1 : 0);

      // If we have a tracking source, turn on the Slice Intersections feature.
      vtkSlicerApplicationGUI *appGUI = this->GetGUI()->GetApplicationGUI();
      if (appGUI)
        {
        int cnnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
        for (int i = 0; i < cnnodes; i++)
          {
          vtkMRMLSliceCompositeNode *cnode = 
            vtkMRMLSliceCompositeNode::SafeDownCast( appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
          cnode->SetSliceIntersectionVisibility( vis );
          }
        }

      // Slice Driven by Locator
      if (transNode)
        {
        vtkMatrix4x4* transform = transNode->GetMatrixTransformToParent();
        if (transform)
          {
          // transform->Print(cerr);
          for (int i = 0; i < 3; i++)
            {
            UpdateSliceNode(i, transform);
            }
          }
        }
      }
        
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}



void vtkBrainlabModuleNavigationStep::Timer(int yes)
{
  this->TimerFlag = yes;
  if (yes)
    {
    this->ProcessTimerEvents();
    }
}



//---------------------------------------------------------------------------
void vtkBrainlabModuleNavigationStep::UpdateSliceNode(int sliceNodeNumber, vtkMatrix4x4* transform)
{

  // NOTES: In Slicer3 ver. 3.2 and higher, a slice orientation information in
  // a slice nodes classes are automaticall set to "Reformat", whenever
  // SetSliceToRASByNTP() function is called.
  // The OpenIGTLinkIF module saves the slice orientations in SliceOrientation[]
  // before the slice nodes update slice orientation information.

  /*
  if (this->FreezePlane)
    {
    return;
    }
  */

  CheckSliceNode();

  float tx = transform->GetElement(0, 0);
  float ty = transform->GetElement(1, 0);
  float tz = transform->GetElement(2, 0);
  /*
  float sx = transform->GetElement(0, 1);
  float sy = transform->GetElement(1, 1);
  float sz = transform->GetElement(2, 1);
  */
  float nx = transform->GetElement(0, 2);
  float ny = transform->GetElement(1, 2);
  float nz = transform->GetElement(2, 2);
  float px = transform->GetElement(0, 3);
  float py = transform->GetElement(1, 3);
  float pz = transform->GetElement(2, 3);

  if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Axial") == 0 &&
      this->AxialCheckButton->GetSelectedState()) 
    {
    if (this->EnableOblique) // perpendicular
      {
      //this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_PERP;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, sliceNodeNumber);
      }
    else
      {
      // cerr << "axial here: " << px << "  "  << py << "  " << pz << endl;
      this->SliceNode[sliceNodeNumber]->SetOrientationToAxial();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }
  else if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Sagittal") == 0 &&
           this->SagittalCheckButton->GetSelectedState()) 
    {
    if (this->EnableOblique) // In-Plane
      {
      //this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_INPLANE;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, sliceNodeNumber);
      }
    else
      {
      this->SliceNode[sliceNodeNumber]->SetOrientationToSagittal();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }
  else if (strcmp(this->SliceNode[sliceNodeNumber]->GetOrientationString(), "Coronal") == 0 &&
           this->CoronalCheckButton->GetSelectedState()) 
    {
    if (this->EnableOblique)  // In-Plane 90
      {
       //this->SliceOrientation[sliceNodeNumber] = SLICE_RTIMAGE_INPLANE90;
      this->SliceNode[sliceNodeNumber]->SetSliceToRASByNTP(nx, ny, nz, tx, ty, tz, px, py, pz, sliceNodeNumber);
      }
    else
      {
      this->SliceNode[sliceNodeNumber]->SetOrientationToCoronal();
      this->SliceNode[sliceNodeNumber]->JumpSlice(px, py, pz);
      }
    }

  this->SliceNode[sliceNodeNumber]->UpdateMatrices();
}



//---------------------------------------------------------------------------
void vtkBrainlabModuleNavigationStep::CheckSliceNode()
{
  
  if (this->SliceNode[0] == NULL)
    {
    this->SliceNode[0] = this->GetGUI()->GetApplicationLogic()
      ->GetSliceLogic("Red")->GetSliceNode();
    }
  if (this->SliceNode[1] == NULL)
    {
    this->SliceNode[1] = this->GetGUI()->GetApplicationLogic()
      ->GetSliceLogic("Yellow")->GetSliceNode();
    }
  if (this->SliceNode[2] == NULL)
    {
    this->SliceNode[2] = this->GetGUI()->GetApplicationLogic()
      ->GetSliceLogic("Green")->GetSliceNode();
    }
}


