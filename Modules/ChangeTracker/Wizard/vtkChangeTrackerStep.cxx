#include "vtkChangeTrackerStep.h"
#include "vtkChangeTrackerGUI.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkCallbackCommand.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerVolumesLogic.h" 
#include "vtkSlicerVolumesGUI.h" 
#include "vtkChangeTrackerLogic.h"

#include "vtkVolumeTextureMapper3D.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkVolumeRayCastCompositeFunction.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerStep);
vtkCxxRevisionMacro(vtkChangeTrackerStep, "$Revision: 1.2 $");
vtkCxxSetObjectMacro(vtkChangeTrackerStep,GUI,vtkChangeTrackerGUI);

//----------------------------------------------------------------------------
vtkChangeTrackerStep::vtkChangeTrackerStep()
{
  this->GUI = NULL;
  this->Frame           = NULL;
  this->AdvancedFrame = NULL;
  this->NextStep = NULL; 
  this->WizardGUICallbackCommand = vtkCallbackCommand::New();
  this->WizardGUICallbackCommand->SetClientData(reinterpret_cast<void *>(this));
  this->GridButton = NULL;
  this->ResetButton = NULL;
  this->SliceButton = NULL;

  this->Render_Image = NULL;
  this->Render_Mapper = NULL;
  this->Render_RayCast_Mapper = NULL;
  this->Render_Filter = NULL;
  this->Render_ColorMapping = NULL;
  this->Render_VolumeProperty = NULL;
  this->Render_Volume = NULL;
  this->Render_OrientationMatrix = NULL; 
}

//----------------------------------------------------------------------------
vtkChangeTrackerStep::~vtkChangeTrackerStep()
{
  this->SetGUI(NULL);
  if (this->Frame)
  {
    this->Frame->Delete();
    this->Frame = NULL;
  }

  if(this->AdvancedFrame)
    {
    this->AdvancedFrame->Delete();
    this->AdvancedFrame = NULL;
    }

  if(this->WizardGUICallbackCommand) 
  {
        this->WizardGUICallbackCommand->Delete();
        this->WizardGUICallbackCommand=NULL;
  }

  if (this->GridButton)
    {
      this->GridButton->Delete();
      this->GridButton = NULL;
    }


  if (this->ResetButton)
    {
      this->ResetButton->Delete();
      this->ResetButton = NULL;
    }

  if (this->SliceButton) {
    this->SliceButton->Delete();
    this->SliceButton = NULL;
    
  } 

//  std::cerr << "RenderRemove in destructor" << std::endl;
  this->RenderRemove();

}

void vtkChangeTrackerStep::RenderHide()
{
  if (this->Render_Volume) {
    if (this->GetGUI()) {
      vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
      if (applicationGUI) {  
        vtkSlicerViewerWidget *viewerWidget  = applicationGUI->GetActiveViewerWidget();
        if (viewerWidget) {
          vtkKWRenderWidget* mainViewer = viewerWidget->GetMainViewer();
          if (mainViewer) {
            mainViewer->RemoveViewProp(this->Render_Volume);
          }
        }
      }
    }
  }
}

void vtkChangeTrackerStep::RenderShow()
{
  if (this->Render_Volume) {
    if (this->GetGUI()) {
      vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
      if (applicationGUI) {  
        vtkSlicerViewerWidget *viewerWidget  = applicationGUI->GetActiveViewerWidget();
        if (viewerWidget) {
          vtkKWRenderWidget* mainViewer = viewerWidget->GetMainViewer();
          if (mainViewer) {
            mainViewer->AddViewProp(this->Render_Volume);
          }
        }
      }
    }
  }
}

void vtkChangeTrackerStep::RenderRemove() { 
//  std::cerr << "Removing render results" << std::endl;
  if (this->Render_Volume) {
    if (this->GetGUI()) {
      vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
      if (applicationGUI) {  
        vtkSlicerViewerWidget *viewerWidget  = applicationGUI->GetActiveViewerWidget();
        if (viewerWidget) {
          vtkKWRenderWidget* mainViewer = viewerWidget->GetMainViewer();
          if (mainViewer) {
            mainViewer->RemoveViewProp(this->Render_Volume);
          }
        }
      }
    }

    this->Render_Volume->Delete();
    this->Render_Volume = NULL; 
  }

  if (this->Render_Mapper) {
    this->Render_Mapper->Delete();
    this->Render_Mapper = NULL;
  }

  if (this->Render_RayCast_Mapper) {
    this->Render_RayCast_Mapper->Delete();
    this->Render_RayCast_Mapper = NULL;
  }

  if (this->Render_Filter) {
    this->Render_Filter->Delete();
    this->Render_Filter = NULL;
  }
  if (this->Render_ColorMapping) {
    this->Render_ColorMapping->Delete();
    this->Render_ColorMapping = NULL;
  }
  if (this->Render_VolumeProperty) {
    this->Render_VolumeProperty->Delete();
    this->Render_VolumeProperty = NULL;
  }

  if (this->Render_OrientationMatrix) {
    this->Render_OrientationMatrix->Delete();
    this->Render_OrientationMatrix = NULL; 
  }
  this->Render_Image = NULL;
}

//----------------------------------------------------------------------------
void vtkChangeTrackerStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  if (this->GetGUI())
    {
    this->GetGUI()->GetWizardWidget()->ClearPage();
    }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerStep::Validate()
{
  this->Superclass::Validate();

  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();

  wizard_workflow->PushInput(vtkKWWizardStep::GetValidationSucceededInput());
  wizard_workflow->ProcessInputs();
}

//----------------------------------------------------------------------------
int vtkChangeTrackerStep::CanGoToSelf()
{
  return this->Superclass::CanGoToSelf() || 1;
}

void vtkChangeTrackerStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();
  
  if (this->NextStep) { this->NextStep->RemoveResults(); }


  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
     wizard_widget->GetCancelButton()->SetEnabled(0);
  vtkKWWidget *parent = wizard_widget->GetClientArea();

  if (!this->Frame)
  {
    this->Frame = vtkKWFrameWithLabel::New();
  }
  if (!this->Frame->IsCreated())
    {
    this->Frame->SetParent(parent);
    this->Frame->Create();
    this->Frame->AllowFrameToCollapseOff();
  }

  if (!this->AdvancedFrame)
  {
    this->AdvancedFrame = vtkKWFrameWithLabel::New();
  }
  if (!this->AdvancedFrame->IsCreated())
    {
    this->AdvancedFrame->SetParent(parent);
    this->AdvancedFrame->Create();
    this->AdvancedFrame->AllowFrameToCollapseOn();
  }

  wizard_widget->NextButtonVisibilityOff();
  wizard_widget->CancelButtonVisibilityOn();
  wizard_widget->GetCancelButton()->SetText("Next >");
  wizard_widget->GetCancelButton()->SetCommand(this, "TransitionCallback");
  wizard_widget->GetCancelButton()->EnabledOn();

  // Does not work 
  // wizard_widget->GetBackButton()->SetCommand(this, "TransitionToPreviousStep");
  // OK Button only is shown at the end  
}
//----------------------------------------------------------------------------
void vtkChangeTrackerStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


void  vtkChangeTrackerStep::GridCallback() {
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return;
 
  vtkMRMLScalarVolumeNode* currentNode =  vtkMRMLScalarVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetGrid_Ref()));
  if (currentNode) {
    this->GridRemove();
    this->GridButton->SetReliefToRidge();
  }
  else if (this->GridDefine()) {
    this->GridButton->SetReliefToSunken();
  }
  this->GetGUI()->PropagateVolumeSelection();
}

void vtkChangeTrackerStep::CreateGridButton() {
  // Grid Button 
  if (!this->GridButton) {
     this->GridButton = vtkKWPushButton::New();
  }

  if (!this->GridButton->IsCreated()) {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    this->GridButton->SetParent(wizard_widget->GetCancelButton()->GetParent());
    this->GridButton->Create();
    this->GridButton->SetWidth(wizard_widget->GetCancelButton()->GetWidth());
    this->GridButton->SetCommand(this, "GridCallback"); 
    this->GridButton->SetText("Grid");
    this->GridButton->SetBalloonHelpString("Show original voxel grid in viewer");
  }
  this->Script("pack %s -side left -anchor nw -expand n -padx 0 -pady 2", this->GridButton->GetWidgetName()); 

  // Button is hold down if Grid already exists 
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return;

  vtkMRMLScalarVolumeNode* currentNode =  vtkMRMLScalarVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetGrid_Ref()));
  if (currentNode) {
    this->GridButton->SetReliefToSunken(); 
  }
}
  
void vtkChangeTrackerStep::GridRemove() {
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (Node) {
    vtkMRMLScalarVolumeNode* currentNode =  vtkMRMLScalarVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetGrid_Ref()));
    if (currentNode) this->GetGUI()->GetMRMLScene()->RemoveNode(currentNode); 
    vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
    applicationLogic->GetSelectionNode()->SetReferenceActiveLabelVolumeID(NULL);
//    applicationLogic->PropagateVolumeSelection( 0 );
    Node->SetGrid_Ref(NULL);
  }
}


int vtkChangeTrackerStep::GridDefine() {
  // Initialize
  this->GridRemove();

  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return 0 ;

  vtkMRMLScene* mrmlScene       =  Node->GetScene();
  vtkMRMLNode* mrmlScan1Node  =  mrmlScene->GetNodeByID(Node->GetScan1_Ref());
  vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(mrmlScan1Node);
  if (!volumeNode) return 0;
  
  vtkSlicerApplication    *application   = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkSlicerVolumesGUI     *volumesGUI    = vtkSlicerVolumesGUI::SafeDownCast(application->GetModuleGUIByName("Volumes")); 
  vtkSlicerVolumesLogic   *volumesLogic  = volumesGUI->GetLogic();
  vtkMRMLScalarVolumeNode *GridNode      = volumesLogic->CreateLabelVolume(mrmlScene,volumeNode, "TG_Grid");
  Node->SetGrid_Ref(GridNode->GetID());

  vtkSlicerSliceControllerWidget *ControlWidget = this->GetGUI()->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceController();
  double oldOffset = ControlWidget->GetOffsetScale()->GetValue(); 
  vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
  applicationLogic->GetSelectionNode()->SetReferenceActiveLabelVolumeID(GridNode->GetID());
  applicationLogic->PropagateVolumeSelection( 0 ); 
  ControlWidget->GetOffsetScale()->SetValue(oldOffset); 

  return 1;
}

void vtkChangeTrackerStep::CreateResetButton() {
  // Grid Button 
  if (!this->ResetButton) {
     this->ResetButton = vtkKWPushButton::New();
  }

  if (!this->ResetButton->IsCreated()) {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    this->ResetButton->SetParent(wizard_widget->GetCancelButton()->GetParent());
    this->ResetButton->Create();
    this->ResetButton->SetWidth(wizard_widget->GetCancelButton()->GetWidth());
    this->ResetButton->SetCommand(this->GetGUI(), "PropagateVolumeSelection"); 
    this->ResetButton->SetText("Reset 3D Viewer");
  }
  this->Script("pack %s -side left -anchor nw -expand n -padx 0 -pady 2", this->ResetButton->GetWidgetName()); 
}

void vtkChangeTrackerStep::CreateSliceButton() {
  if (!this->SliceButton) {
     this->SliceButton = vtkKWPushButton::New();
  }

  if (!this->SliceButton->IsCreated()) {
    vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
    this->SliceButton->SetParent(wizard_widget->GetCancelButton()->GetParent());
    this->SliceButton->Create();
    this->SliceButton->SetWidth(wizard_widget->GetCancelButton()->GetWidth());
    this->SliceButton->SetCommand(this, "SliceCallback"); 
    this->SliceButton->SetText("Slice");
    this->SliceButton->SetBalloonHelpString("Show Axial view of complete slice in 3D Viewer");
  }
  this->Script("pack %s -side left -anchor nw -expand n -padx 0 -pady 2", this->SliceButton->GetWidgetName()); 
  if (this->GetGUI()->GetSliceLogic()) this->SliceButton->SetReliefToSunken();   
}

void  vtkChangeTrackerStep::SliceCallback() { 
  if (this->GetGUI()->GetSliceLogic()) {
    this->GetGUI()->SliceLogicRemove(); 
    this->SliceButton->SetReliefToRidge();
  }
  else {
    this->GetGUI()->SliceLogicDefine(); 
    this->SliceButton->SetReliefToSunken();
  }
}

/// For Rendering results
void vtkChangeTrackerStep::ChangeRender_BandPassFilter(double min, double max) {
  double* imgRange  =   this->Render_Image->GetPointData()->GetScalars()->GetRange();
  
  this->Render_Filter->RemoveAllPoints();
  this->Render_Filter->AddPoint(imgRange[0], 0.0);
  this->Render_Filter->AddPoint(min - .1, 0.0);
  this->Render_Filter->AddPoint(min, 1., .5, 1.);
//  this->Render_Filter->AddPoint(min+0.1, 0.);
//  this->Render_Filter->AddPoint(max-0.1, 0.);
  this->Render_Filter->AddPoint(max, 1., .5, 1.);
  if (max < imgRange[1]) { 
    this->Render_Filter->AddPoint(max + .1, 0.);
    if (max+.1 < imgRange[1]) { 
      this->Render_Filter->AddPoint(max+.1, 0.);
      this->Render_Filter->AddPoint(imgRange[1], 0);
    }
  }
  this->Render_Filter->ClampingOff();
}

void vtkChangeTrackerStep::SetRender_BandPassFilter(double min, double max, float colorMin[3], float colorMax[3]) {
  // cout <<  "SetPreSegment_Render_BandPassFilter " << value << endl;
  this->ChangeRender_BandPassFilter(min, max);
  this->Render_ColorMapping->RemoveAllPoints();
  // Two different colors did not work 
  this->Render_ColorMapping->AddRGBPoint(min, colorMin[0], colorMin[1], colorMin[2]);
  this->Render_ColorMapping->AddRGBPoint(max, colorMax[0], colorMax[1], colorMax[2]);
}

// Set the opacity to show only the two specified levels of intensity
void vtkChangeTrackerStep::SetRender_PulsePassFilter(double p0, double p1, float color_p0[3], float color_p1[3]) {
  double* imgRange  =   this->Render_Image->GetPointData()->GetScalars()->GetRange();
  
  this->Render_Filter->RemoveAllPoints();
  this->Render_Filter->AddPoint(imgRange[0], 0.0);
  this->Render_Filter->AddPoint(p0-.1, 0.0);
  this->Render_Filter->AddPoint(p0, 1., .5, 1.);
  this->Render_Filter->AddPoint(p0+0.1, 0.);
  this->Render_Filter->AddPoint(p1-0.1, 0.);
  this->Render_Filter->AddPoint(p1, 1., .5, 1.);
  if (p1 < imgRange[1]) { 
    this->Render_Filter->AddPoint(p1+.1, 0.);
    if (p1+.1 < imgRange[1]) { 
      this->Render_Filter->AddPoint(p1+.1, 0.);
      this->Render_Filter->AddPoint(imgRange[1], 0);
    }
  }
  this->Render_Filter->ClampingOff();

  this->Render_ColorMapping->RemoveAllPoints();
  // Two different colors did not work 
  this->Render_ColorMapping->AddRGBPoint(p0, color_p0[0], color_p0[1], color_p0[2]);
  this->Render_ColorMapping->AddRGBPoint(p1, color_p1[0], color_p1[1], color_p1[2]);
}

void vtkChangeTrackerStep::SetRender_HighPassFilter(double min, float colorMin[3], float colorMax[3]) {
  // cout <<  "SetPreSegment_Render_BandPassFilter " << value << endl;
  double* imgRange  =   this->Render_Image->GetPointData()->GetScalars()->GetRange();
  this->Render_Filter->RemoveAllPoints();
  this->Render_Filter->AddPoint(imgRange[0], 0.0);
  this->Render_Filter->AddPoint(min - 1, 0.0);
  this->Render_Filter->AddPoint(min, 1);

  this->Render_ColorMapping->RemoveAllPoints();
  this->Render_ColorMapping->AddRGBPoint(min,  colorMin[0], colorMin[1], colorMin[2]);
  this->Render_ColorMapping->AddRGBPoint(imgRange[1] , colorMax[0], colorMax[1], colorMax[2]);

}

void vtkChangeTrackerStep::SetRender_BandStopFilter(double min, double max, float colorMin[3], float colorMax[3]) {
  // cout <<  "SetPreSegment_Render_BandPassFilter " << value << endl;
  double* imgRange  =   this->Render_Image->GetPointData()->GetScalars()->GetRange();
  this->Render_Filter->RemoveAllPoints();
  this->Render_Filter->AddPoint(imgRange[0], 0.8);
  this->Render_Filter->AddPoint(min-0.01,0.8);
  this->Render_Filter->AddPoint(min,0);
  this->Render_Filter->AddPoint(max,0);
  this->Render_Filter->AddPoint(max+0.01,0.8);
  this->Render_Filter->AddPoint(imgRange[1],0.8);

  this->Render_ColorMapping->RemoveAllPoints();
  this->Render_ColorMapping->AddRGBPoint(imgRange[0] , colorMin[0], colorMin[1], colorMin[2]);
  this->Render_ColorMapping->AddRGBPoint(imgRange[1] , colorMax[0], colorMax[1], colorMax[2]);
}


void vtkChangeTrackerStep::CreateRender(vtkMRMLVolumeNode *volumeNode, int RayCastFlag ) {
//  std::cerr << "RenderRemove in CreateRender" << std::endl;
  this->RenderRemove();
  if (!volumeNode) return;

  this->Render_Image = volumeNode->GetImageData();
  
  // set PROP [[vtkChangeTrackerAnalysisStep ListInstances] GetRender_Mapper]
  if (RayCastFlag ) {
    this->Render_RayCast_Mapper = vtkFixedPointVolumeRayCastMapper::New();
    this->Render_RayCast_Mapper->SetInput(this->Render_Image);
    this->Render_RayCast_Mapper->SetAutoAdjustSampleDistances(0);
    this->Render_RayCast_Mapper->SetSampleDistance(0.1);
  } else {
    this->Render_Mapper = vtkVolumeTextureMapper3D::New();
    this->Render_Mapper->SetInput(this->Render_Image);
  }
  this->Render_Filter = vtkPiecewiseFunction::New();
  this->Render_ColorMapping = vtkColorTransferFunction::New();

  // set PROP [[vtkChangeTrackerAnalysisStep ListInstances] GetRender_VolumeProperty]

  this->Render_VolumeProperty = vtkVolumeProperty::New();
  this->Render_VolumeProperty->SetShade(1);
  this->Render_VolumeProperty->SetAmbient(0.3);
  this->Render_VolumeProperty->SetDiffuse(0.6);
  this->Render_VolumeProperty->SetSpecular(0.5);
  this->Render_VolumeProperty->SetSpecularPower(40.0);
  this->Render_VolumeProperty->SetScalarOpacity(this->Render_Filter);
  this->Render_VolumeProperty->SetColor( this->Render_ColorMapping );
  this->Render_VolumeProperty->SetInterpolationTypeToNearest();
  this->Render_VolumeProperty->ShadeOn();

  this->Render_OrientationMatrix = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(this->Render_OrientationMatrix);

  this->Render_Volume = vtkVolume::New();
  this->Render_Volume->SetProperty(this->Render_VolumeProperty);
  if (RayCastFlag) {
    this->Render_Volume->SetMapper(this->Render_RayCast_Mapper);
  } else {
    this->Render_Volume->SetMapper(this->Render_Mapper);
  }

  this->Render_Volume->PokeMatrix(this->Render_OrientationMatrix);
  
  vtkSlicerViewerWidget *active_viewer = 
    this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget();
  active_viewer->GetMainViewer()->AddViewProp(this->Render_Volume);
}

void vtkChangeTrackerStep::CenterRYGSliceViews(double ptX, double ptY, double ptZ) 
{
      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
      applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->SetValue(ptZ);
      applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->SetValue(ptX);
      applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->SetValue(ptY);
}
