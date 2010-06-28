#include "vtkChangeTrackerROIStep.h"

#include "vtkChangeTrackerGUI.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWMessageDialog.h"
#include "vtkImageRectangularSource.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerVolumesGUI.h" 
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWScale.h"

//#include "vtkKWMatrixWidget.h"
#include "vtkMRMLROINode.h"
#include <sstream>
#include "vtkObserverManager.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkPiecewiseFunction.h"
#include "vtkSlicerROIDisplayWidget.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSetWithLabel.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWSpinBox.h"
#include "vtkKWRadioButton.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerColorLogic.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerROIStep);
vtkCxxRevisionMacro(vtkChangeTrackerROIStep, "$Revision: 1.2 $");

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------------
vtkChangeTrackerROIStep::vtkChangeTrackerROIStep()
{
  this->SetName("2/4. Define Volume of Interest"); 
  this->SetDescription("Click right mouse button to re-center VOI, use left button to resize"); 
  this->WizardGUICallbackCommand->SetCallback(vtkChangeTrackerROIStep::WizardGUICallback);

  this->FrameButtons    = NULL;
  this->FrameBlank      = NULL;
  this->FrameROI        = NULL;
  this->FrameROIX       = NULL;
  this->FrameROIY       = NULL;
  this->FrameROIZ       = NULL;
  this->ButtonsShow     = NULL;
  this->ButtonsReset    = NULL;
  this->ROIX            = NULL;
  this->ROIY            = NULL;
  this->ROIZ            = NULL;
  this->LabelROIX       = NULL;
  this->LabelROIY       = NULL;
  this->LabelROIZ       = NULL;
  this->ROILabelMapNode = NULL;
  this->ROILabelMap     = NULL;
  this->ROIHideFlag     = 0; 

  this->roiNode = NULL;
  this->roiWidget = NULL;
  this->roiUpdateGuard = false;
  this->FrameROIIJK = NULL;

  this->ResamplingChoice = NULL;
  this->SpinResampleConst = NULL;

  this->ROIMRMLCallbackCommand = vtkCallbackCommand::New();
  this->ROIMRMLCallbackCommand->SetClientData(reinterpret_cast<void*>(this));
  this->ROIMRMLCallbackCommand->SetCallback(vtkChangeTrackerROIStep::ROIMRMLCallback);

  this->VisibilityIcons = NULL;
  this->VisibilityLabel = NULL;
}

//----------------------------------------------------------------------------
vtkChangeTrackerROIStep::~vtkChangeTrackerROIStep()
{
  if (this->FrameButtons)
  {
    this->FrameButtons->Delete();
    this->FrameButtons = NULL;
  }

  if (this->FrameBlank)
  {
    this->FrameBlank->Delete();
    this->FrameBlank = NULL;
  }

  if (this->FrameROI)
  {
    this->FrameROI->Delete();
    this->FrameROI = NULL;
  }

  if (this->FrameROIIJK)
    {
    this->FrameROIIJK->Delete();
    this->FrameROIIJK = NULL;
    }

  if (this->FrameROIX)
  {
    this->FrameROIX->Delete();
    this->FrameROIX = NULL;
  }

  if (this->FrameROIY)
  {
    this->FrameROIY->Delete();
    this->FrameROIY = NULL;
  }

  if (this->FrameROIZ)
  {
    this->FrameROIZ->Delete();
    this->FrameROIZ = NULL;
  }
  if (this->ROIX)
  {
    this->ROIX->Delete();
    this->ROIX = NULL;
  }

  if (this->ROIY)
  {
    this->ROIY->Delete();
    this->ROIY = NULL;
  }

  if (this->ROIZ)
  {
    this->ROIZ->Delete();
    this->ROIZ = NULL;
  }

  if (this->LabelROIX)
  {
    this->LabelROIX->Delete();
    this->LabelROIX = NULL;
  }

  if (this->LabelROIY)
  {
    this->LabelROIY->Delete();
    this->LabelROIY = NULL;
  }

  if (this->LabelROIZ)
  {
    this->LabelROIZ->Delete();
    this->LabelROIZ = NULL;
  }

  if (this->ButtonsShow) {
    this->ButtonsShow->Delete();
    this->ButtonsShow= NULL;
  }

  if (this->ButtonsReset) {
    this->ButtonsReset->Delete();
    this->ButtonsReset= NULL;
  }

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove();

  if(this->roiNode)
  {
    this->roiNode->Delete();
    this->roiNode = NULL;
  }

  if(this->roiWidget)
    {
    this->roiWidget->Delete();
    this->roiWidget = NULL;
    }

  if(this->ResamplingChoice)
    {
    this->ResamplingChoice->Delete();
    this->ResamplingChoice = NULL;
    }

  if (this->SpinResampleConst)
    {
    this->SpinResampleConst->Delete();
    this->SpinResampleConst = NULL;
    }
  if (this->ROIMRMLCallbackCommand)
    {
    this->ROIMRMLCallbackCommand->Delete();
    this->ROIMRMLCallbackCommand = NULL;
    }
  if ( this->VisibilityIcons ) {
    this->VisibilityIcons->Delete  ( );
    this->VisibilityIcons = NULL;
    }
  if ( this->VisibilityLabel ) {
    this->VisibilityLabel->SetParent(NULL);
    this->VisibilityLabel->Delete();
    this->VisibilityLabel = NULL;
  }

}

void vtkChangeTrackerROIStep::DeleteSuperSampleNode() 
{
  this->GetGUI()->GetLogic()->DeleteSuperSample(1);
} 

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::ShowUserInterface()
{
//  cout << "vtkChangeTrackerROIStep::ShowUserInterface() Start " << endl;
  // ----------------------------------------
  // Display Scan1, Delete Super Sampled and Grid  
  // ----------------------------------------
  this->DeleteSuperSampleNode();

  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  int dimensions[3]={1,1,1};
  if (node) {
    vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_Ref()));
    if (volumeNode) {

      /*
      vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();
      applicationLogic->GetSelectionNode()->SetActiveVolumeID(volumeNode->GetID());
      applicationLogic->PropagateVolumeSelection(); 
      */
      if (!volumeNode->GetImageData()) {
        vtkKWMessageDialog::PopupMessage(this->GetGUI()->GetApplication(), 
                                         this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow(),
                                         "Change Tracker", 
                                         "No image data associated with Scan 1", 
                                         vtkKWMessageDialog::ErrorIcon);
        return;
      }

      memcpy(dimensions,volumeNode->GetImageData()->GetDimensions(),sizeof(int)*3);

      this->GetGUI()->SetRedGreenYellowAllVolumes(volumeNode->GetID(), "", "");
      // Load File 
      //char fileName[1024];
      //sprintf(fileName,"%s/TG_Analysis_Intensity.nhdr",node->GetWorkingDir());

      // vtkMRMLVolumeNode* tmp =  this->GetGUI()->GetLogic()->LoadVolume(vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication()),fileName,1,"TG_analysis");
    } 
  } else {
    cout << "no node "  << endl;
  }

  this->GridRemove();

  // ----------------------------------------
  // Build GUI 
  // ----------------------------------------

  this->vtkChangeTrackerStep::ShowUserInterface();
  // Create the frame
  // Needs to be check bc otherwise with wizrd can be created over again

  this->Frame->SetLabelText("Define VOI");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->Frame->GetWidgetName());

  if (!this->FrameButtons)
    {
    this->FrameButtons = vtkKWFrame::New();
    }
  if (!this->FrameButtons->IsCreated())
    {
      this->FrameButtons->SetParent(this->Frame->GetFrame());
    this->FrameButtons->Create();
    // this->FrameButtons->SetLabelText("");
    // define buttons 
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameButtons->GetWidgetName());
  if (!this->FrameBlank)
    {
    this->FrameBlank = vtkKWFrame::New();
    }
  if (!this->FrameBlank->IsCreated())
    {
      this->FrameBlank->SetParent(this->Frame->GetFrame());
    this->FrameBlank->Create();
    // this->FrameButtons->SetLabelText("");
    // define buttons 
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 4", this->FrameBlank->GetWidgetName());

  if (!this->FrameROI)
    {
    this->FrameROI = vtkSlicerModuleCollapsibleFrame::New();
    }
  if (!this->FrameROI->IsCreated())
    {
      this->FrameROI->SetParent(this->Frame->GetFrame());
    this->FrameROI->Create();
    this->FrameROI->SetLabelText("ROI Widget controls: RAS Space");
    // this->FrameROI->CollapseFrame();
  }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameROI->GetWidgetName());

#ifdef SHOWROIIJK
  if (!this->FrameROIIJK)
    {
    this->FrameROIIJK = vtkSlicerModuleCollapsibleFrame::New();
    }
  if (!this->FrameROIIJK->IsCreated())
    {
    this->FrameROIIJK->SetParent(this->Frame->GetFrame());
    this->FrameROIIJK->Create();
    this->FrameROIIJK->SetLabelText("ROI Widget controls: IJK Space");
    this->FrameROIIJK->CollapseFrame();
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameROIIJK->GetWidgetName());
#endif // SHOWROIIJK

  if (!this->ButtonsShow) {
    this->ButtonsShow = vtkKWPushButton::New();
  }

  if(!this->VisibilityIcons)
    this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );

  if (!this->ButtonsShow->IsCreated()) {
    this->ButtonsShow->SetParent(this->FrameButtons);
    this->ButtonsShow->Create();
//    this->ButtonsShow->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH);
    this->ButtonsShow->SetText("Show render");
    this->ButtonsShow->SetReliefToFlat ( );
    this->ButtonsShow->SetOverReliefToNone ( );
    this->ButtonsShow->SetBorderWidth ( 0 );
    this->ButtonsShow->SetBalloonHelpString("Show/hide VOI rendering and label overlay in image viewer"); 
    this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());
  }
  if(!this->VisibilityLabel){
    this->VisibilityLabel = vtkKWLabel::New();
  }
  if(!this->VisibilityLabel->IsCreated()){
    this->VisibilityLabel->SetParent (this->FrameButtons);
    this->VisibilityLabel->Create();
    this->VisibilityLabel->SetText("Display volume rendering and VOI label");
  }

  if (!this->ButtonsReset) {
    this->ButtonsReset = vtkKWPushButton::New();
  }
  if (!this->ButtonsReset->IsCreated()) {
    this->ButtonsReset->SetParent(this->FrameButtons);
    this->ButtonsReset->Create();
    this->ButtonsReset->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH);
    this->ButtonsReset->SetText("Reset");
    this->ButtonsReset->SetBalloonHelpString("Reset Values"); 
  }

  this->Script("pack %s %s %s -side left -anchor w -padx 4 -pady 2 -in %s", 
                this->VisibilityLabel->GetWidgetName(), this->ButtonsShow->GetWidgetName(),
                this->ButtonsReset->GetWidgetName(), this->FrameButtons->GetWidgetName());

#ifdef SHOWROIIJK
  if (!this->FrameROIX)
    {
    this->FrameROIX = vtkKWFrame::New();
    }
  if (!this->FrameROIX->IsCreated())
    {
      this->FrameROIX->SetParent(this->FrameROIIJK->GetFrame());
    this->FrameROIX->Create();
  }

  if (!this->LabelROIX)
    {
    this->LabelROIX = vtkKWLabel::New();
    }
  if (!this->LabelROIX->IsCreated())
    {

    this->LabelROIX->SetParent(this->FrameROIX);
    this->LabelROIX->Create();
    this->LabelROIX->SetText("X:");

    }
  if (!this->ROIX)
    {
    this->ROIX = vtkKWRange::New();
    }
  if (!this->ROIX->IsCreated())
    {

    this->ROIX->SetParent(this->FrameROIX);
    this->ROIX->Create();
    this->ROIX->SymmetricalInteractionOff();
//    this->ROIX->SetCommand(this, "ROIXChangedCallback");    
    this->ROIX->SetWholeRange(-1, double(dimensions[0]-1));
    this->ROIX->SetResolution(1);
    }

  if (!this->FrameROIY)
    {
    this->FrameROIY = vtkKWFrame::New();
    }
  if (!this->FrameROIY->IsCreated())
    {
      this->FrameROIY->SetParent(this->FrameROIIJK->GetFrame());
    this->FrameROIY->Create();
  }

  if (!this->LabelROIY)
    {
    this->LabelROIY = vtkKWLabel::New();
    }
  if (!this->LabelROIY->IsCreated())
    {

    this->LabelROIY->SetParent(this->FrameROIY);
    this->LabelROIY->Create();
    this->LabelROIY->SetText("Y:");

    }

 if (!this->ROIY)
    {
    this->ROIY = vtkKWRange::New();
    }
  if (!this->ROIY->IsCreated())
    {

    this->ROIY->SetParent(this->FrameROIY);

    this->ROIY->Create();
    this->ROIY->SymmetricalInteractionOff();
//    this->ROIY->SetCommand(this, "ROIYChangedCallback");    
    this->ROIY->SetWholeRange(-1, double(dimensions[1]-1));
    this->ROIY->SetResolution(1);
    }

  if (!this->FrameROIZ)
    {
    this->FrameROIZ = vtkKWFrame::New();
    }
  if (!this->FrameROIZ->IsCreated())
    {
      this->FrameROIZ->SetParent(this->FrameROIIJK->GetFrame());
    this->FrameROIZ->Create();
  }

  if (!this->LabelROIZ)
    {
    this->LabelROIZ = vtkKWLabel::New();
    }
  if (!this->LabelROIZ->IsCreated())
    {

    this->LabelROIZ->SetParent(this->FrameROIZ);
    this->LabelROIZ->Create();
    this->LabelROIZ->SetText("Z:");

    }

  if (!this->ROIZ)
    {
    this->ROIZ = vtkKWRange::New();
    }
  if (!this->ROIZ->IsCreated())
    {

    this->ROIZ->SetParent(this->FrameROIZ);

    this->ROIZ->Create();
    this->ROIZ->SymmetricalInteractionOff();
//    this->ROIZ->SetCommand(this, "ROIZChangedCallback");    
    this->ROIZ->SetWholeRange(-1, double(dimensions[2]-1));
    this->ROIZ->SetResolution(1);
    }

  
  this->Script("pack %s %s %s -side top -anchor nw -padx 0 -pady 3",this->FrameROIX->GetWidgetName(),this->FrameROIY->GetWidgetName(),this->FrameROIZ->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIX->GetWidgetName(),this->ROIX->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIY->GetWidgetName(),this->ROIY->GetWidgetName());
  this->Script("pack %s %s -side left -anchor nw -padx 2 -pady 0",this->LabelROIZ->GetWidgetName(),this->ROIZ->GetWidgetName());

#endif // SHOWROIIJK
  
  {
   vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget(); 
   wizard_widget->BackButtonVisibilityOn();
   wizard_widget->GetCancelButton()->EnabledOn();
  }
 
  // Set it up so it has default value from MRML file 
  if(!node->GetROI_Ref())
    {
    // Create ROI MRML node
    // see Base/GUI/vtkSlicerNodeSelectorWidget.cxx:ProcessNewNodeCommand
    vtkMRMLScene *scene = node->GetScene();
    this->roiNode = 
      static_cast<vtkMRMLROINode*>(scene->CreateNodeByClass("vtkMRMLROINode"));
    scene->AddNode(this->roiNode);
    this->roiNode->SetName("ChangeTrackerROI");
    this->roiNode->SetVisibility(0);
    node->SetROI_Ref(this->roiNode->GetID());
    }
  else
    {
    vtkMRMLScene *scene = node->GetScene();
    this->roiNode = static_cast<vtkMRMLROINode*>(scene->GetNodeByID(node->GetROI_Ref()));
    if(!this->roiNode)
      {
      vtkErrorMacro("Invalid reference to ROI in ChangeTracker node. Unable to proceed.");
      return;
      }
    }
  
  // update the ROI label map to reflect what is stored in ROI MRML
  this->MRMLUpdateROIFromROINode();
  this->roiNode->SetVisibility(!this->ROIHideFlag);
  this->roiNode->AddObserver(vtkCommand::ModifiedEvent, this->ROIMRMLCallbackCommand);

  InitROIRender();
//  ResetROIRender();
//  this->MRMLUpdateROINodeFromROI();
  
 
  if (!this->roiWidget)
    {
    this->roiWidget = vtkSlicerROIDisplayWidget::New();
    }

  if (!this->roiWidget->IsCreated())
    {
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
    vtkMRMLSliceLogic *sliceLogic = this->GetGUI()->GetSliceLogic();
    vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
    double rasDimensions[3];
    double rasCenter[3];
    double rasBounds[6];

    sliceLogic->GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);

    rasBounds[0] = min(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
    rasBounds[1] = min(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
    rasBounds[2] = min(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    rasBounds[3] = max(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
    rasBounds[4] = max(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
    rasBounds[5] = max(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    
    this->roiWidget->SetParent(this->FrameROI->GetFrame());
    this->roiWidget->Create();

    this->roiWidget->SetXRangeExtent(rasBounds[0],rasBounds[3]);
    this->roiWidget->SetYRangeExtent(rasBounds[1],rasBounds[4]);
    this->roiWidget->SetZRangeExtent(rasBounds[2],rasBounds[5]);

    this->roiWidget->SetXResolution(fabs(rasBounds[3]-rasBounds[0])/100.);
    this->roiWidget->SetYResolution(fabs(rasBounds[4]-rasBounds[1])/100.);
    this->roiWidget->SetZResolution(fabs(rasBounds[5]-rasBounds[2])/100.);

    this->roiWidget->SetROINode(roiNode);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 3 -fill x",
               this->roiWidget->GetWidgetName());

  this->AdvancedFrame->SetLabelText("Advanced settings");
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
    this->AdvancedFrame->GetWidgetName());

  if(!this->ResamplingChoice)
    {
    this->ResamplingChoice = vtkKWRadioButtonSetWithLabel::New();
    }

  if(!this->ResamplingChoice->IsCreated())
    {
    this->ResamplingChoice->SetParent(this->AdvancedFrame->GetFrame());
    this->ResamplingChoice->Create();
    this->ResamplingChoice->SetLabelText("ROI resampling:");
    this->ResamplingChoice->SetBalloonHelpString("Describe if and how the iput data ROI should be resampled");
//    this->ResamplingChoice->GetWidget()->PackVerticallyOn();

    // now add a widget for each of the options
    vtkKWRadioButton *rc0 = this->ResamplingChoice->GetWidget()->AddWidget(0);
    rc0->SetValue("0");
    rc0->SetText("Do not resample my data");
    rc0->SetBalloonHelpString("ChangeTracker will not resample the image corresponding to the ROI you selected in this step. Choose this if the input images are very large, as the more you resample, the slower registration will become.");
    rc0->SetAnchorToWest();
    rc0->SetSelectedState(0);
    rc0->SetEnabled(0);

    vtkKWRadioButton *rc1 = this->ResamplingChoice->GetWidget()->AddWidget(1);
    rc1->SetValue("1");
    rc1->SetText("Legacy resampling");
    rc1->SetBalloonHelpString("Use the same resampling strategy as in Slicer 3.4 ChangeTracker");
    rc1->SetSelectedState(0);
//    rc1->SetEnabled(0);

    vtkKWRadioButton *rc2 = this->ResamplingChoice->GetWidget()->AddWidget(2);
    rc2->SetValue("2");
    rc2->SetText("Isotropic resampling");
    rc2->SetBalloonHelpString("Resample the ROI to isotropic pixel size equal to the original *minimum* spacing times the constant below");
    rc2->SetAnchorToWest();
    rc2->SetSelectedState(1);
//    rc2->SetEnabled(0);

    if (node) 
      {
      switch(node->GetResampleChoice())
        {
        case RESCHOICE_NONE: rc0->SetSelectedState(1);break;
        case RESCHOICE_LEGACY: rc1->SetSelectedState(1);break;
        case RESCHOICE_ISO: rc2->SetSelectedState(1);break;
        default: std::cerr << "MRML node contains invalid data!";
        }
      }
    }

  if(!this->SpinResampleConst)
    {
    this->SpinResampleConst = vtkKWSpinBoxWithLabel::New();
    }
  if(!this->SpinResampleConst->IsCreated())
    {
    this->SpinResampleConst->SetParent(this->AdvancedFrame->GetFrame());
    this->SpinResampleConst->Create();
    this->SpinResampleConst->SetLabelText("Resampling constant:");
    this->SpinResampleConst->GetWidget()->SetValue(0.5);
    this->SpinResampleConst->SetBalloonHelpString("The value of isotropic sampling will be obtained by multiplying the smallest pixel dimension by the value defined here.");
    }
  
    if (node) 
      {
      this->SpinResampleConst->GetWidget()->SetValue(node->GetResampleConst());
      }


  this->Script("pack %s %s -side top -anchor nw -fill x -padx 2 -pady 2", 
               this->ResamplingChoice->GetWidgetName(),
               this->SpinResampleConst->GetWidgetName());
  this->AdvancedFrame->CollapseFrame();

  // Very Important 
  this->AddGUIObservers();
  // Keep seperate bc GUIObserver is also called from vtkChangeTrackerGUI ! 
  // You only want to add the observers below when the step is active 
  this->AddROISamplingGUIObservers();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::ROIXChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(0,min, max);
}


//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::ROIYChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(1,min, max);
}

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::ROIZChangedCallback(double min, double max)  
{
  this->ROIChangedCallback(2,min, max);
}  

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::ROIChangedCallback(int axis, double min, double max)  
{
  vtkMRMLChangeTrackerNode *mrmlNode = this->GetGUI()->GetNode();
  if (!mrmlNode) return;

  mrmlNode->SetROIMin(axis,int(min));  
  mrmlNode->SetROIMax(axis,int(max));  
  this->ROIMapUpdate();

}



//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::AddGUIObservers() 
{
  // cout << "vtkChangeTrackerROIStep::AddGUIObservers()" << endl; 
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->ButtonsShow && (!this->ButtonsShow->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsShow->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 

  if (this->ButtonsReset && (!this->ButtonsReset->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsReset->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkChangeTrackerROIStep::AddROISamplingGUIObservers() {
  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand, 1);
  rwi0->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand, 1);

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent,this->WizardGUICallbackCommand, 1);
  rwi1->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent,this->WizardGUICallbackCommand, 1);

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand, 1);
  rwi2->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand, 1);
} 


void vtkChangeTrackerROIStep::RemoveGUIObservers() 
{
  if (this->ButtonsShow) 
    {
      this->ButtonsShow->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
    }

  if (this->ButtonsReset) 
  {
      this->ButtonsReset->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  }
  this->RemoveROISamplingGUIObservers();
}


void vtkChangeTrackerROIStep::RemoveROISamplingGUIObservers() {
  if (!this->GetGUI()) return;
  vtkSlicerApplicationGUI *ApplicationGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI());
  if (!ApplicationGUI) return; 
  for (int i = 0 ; i < 3 ; i ++ ) {
    vtkSlicerSliceGUI *MainGUI = NULL;
    if (i == 0) MainGUI = ApplicationGUI->GetMainSliceGUI("Red");
    if (i == 1) MainGUI = ApplicationGUI->GetMainSliceGUI("Yellow");
    if (i == 2) MainGUI = ApplicationGUI->GetMainSliceGUI("Green");
    if (!MainGUI) return;
    vtkRenderWindowInteractor *rwi = MainGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    rwi->GetInteractorStyle()->RemoveObservers(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand);
    rwi->GetInteractorStyle()->RemoveObservers(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand);
  }
}

void vtkChangeTrackerROIStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkChangeTrackerROIStep *self = reinterpret_cast<vtkChangeTrackerROIStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}

void vtkChangeTrackerROIStep::ROIReset() {
  // cout << "ROIReset Start" << endl;
#ifdef SHOWROIIJK
  if (this->ROIX) this->ROIX->SetRange(-1,-1);
  if (this->ROIY) this->ROIY->SetRange(-1,-1);
  if (this->ROIZ) this->ROIZ->SetRange(-1,-1);
#endif // SHOWROIIJK

  this->ROIHideFlag = 0;
  this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());

  if(this->roiNode){
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
    //vtkSlicerSliceLogic *sliceLogic = this->GetGUI()->GetSliceLogic();
    vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
    
    vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
    volumeNode->GetIJKToRASMatrix(ijkToras);
    double pointRAS[4], pointIJK[4];
    pointIJK[0] = -1.;
    pointIJK[1] = -1.;
    pointIJK[2] = -1.;
    pointIJK[3] = 1.;
    ijkToras->MultiplyPoint(pointIJK,pointRAS);
    ijkToras->Delete();

    this->roiNode->SetXYZ(pointRAS[0], pointRAS[1], pointRAS[2]);
    this->roiNode->SetRadiusXYZ(0., 0., 0.);
    this->roiNode->SetVisibility(0);
  }
}


void vtkChangeTrackerROIStep::ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample) {
  if (!ROIAxis) return;
  double *oldRange = ROIAxis->GetRange();
  double newRange[2];

  if ((Sample < oldRange[0]) || (oldRange[0] < 0)) newRange[0] = Sample;
  else  newRange[0] = oldRange[0]; 
  if ((Sample > oldRange[1]) || (oldRange[1] < 0)) newRange[1] = Sample;
  else newRange[1] = oldRange[1]; 
  ROIAxis->SetRange(newRange);
}

void vtkChangeTrackerROIStep::ROIUpdateWithNewSample(double rasSample[3]) {
  double *roiXYZ = roiNode->GetXYZ();
  double *roiRadius = roiNode->GetRadiusXYZ();
  double dXYZ[3], shiftXYZ[3];
  int i;

  for(i=0;i<3;i++)
    {
    dXYZ[i] = fabs(roiXYZ[i]-rasSample[i]);
    shiftXYZ[i] = fabs(dXYZ[i]-roiRadius[i])/2.;
    }

  if(dXYZ[0]>roiRadius[0] || dXYZ[1]>roiRadius[1] || dXYZ[2]>roiRadius[2])
    {
    // click outside the box
    for(i=0;i<3;i++)
      {
      if(dXYZ[i]>roiRadius[i])
        {
        if(rasSample[i]>roiXYZ[i])
          {
          roiXYZ[i] = roiXYZ[i]+shiftXYZ[i];
          }
        else
          {
          roiXYZ[i] = roiXYZ[i]-shiftXYZ[i];
          }
        roiRadius[i] = roiRadius[i]+shiftXYZ[i];
        }
      }
    }
  // in-slice reduction of ROI size is not there yet, because need to pass
  // clickXY to this function
  this->roiNode->SetXYZ(roiXYZ);
  this->roiNode->SetRadiusXYZ(roiRadius);
}

void vtkChangeTrackerROIStep::ROIUpdateAxisWithNode(vtkMRMLChangeTrackerNode* Node, vtkKWRange *ROIAxis, int Axis) {
  if (!Node || !ROIAxis) return;
  ROIAxis->SetRange(Node->GetROIMin(Axis),Node->GetROIMax(Axis));
}

void vtkChangeTrackerROIStep::ROIUpdateWithNode() {
  // cout << "ROIUpdateWithNode Start" << endl;
#if SHOWROIIJK
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  this->ROIUpdateAxisWithNode(Node, this->ROIX,0); 
  this->ROIUpdateAxisWithNode(Node, this->ROIY,1); 
  this->ROIUpdateAxisWithNode(Node, this->ROIZ,2); 
#endif // SHOWROIIJK
  this->ROIMapUpdate();
}



// Return 1 if it is a valid ROI and zero otherwise
int vtkChangeTrackerROIStep::ROICheck() {
  // Define Variables
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) return 0;

  vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
  if (!volumeNode) return 0;
  return this->GetGUI()->GetLogic()->CheckROI(volumeNode);
}

void vtkChangeTrackerROIStep::ROIMapUpdate() {

  vtkMRMLChangeTrackerNode* Node      =  this->GetGUI()->GetNode();
  if (!this->ROILabelMapNode || !this->ROILabelMap || !Node || !this->ROICheck()) return;
  int size[3]   = {Node->GetROIMax(0) - Node->GetROIMin(0) + 1, 
                   Node->GetROIMax(1) - Node->GetROIMin(1) + 1, 
                   Node->GetROIMax(2) - Node->GetROIMin(2) + 1};

  int center[3] = {(Node->GetROIMax(0) + Node->GetROIMin(0))/2,
                   (Node->GetROIMax(1) + Node->GetROIMin(1))/2, 
                   (Node->GetROIMax(2) + Node->GetROIMin(2))/2};

  this->ROILabelMap->SetCenter(center);
  this->ROILabelMap->SetSize(size);
  this->ROILabelMap->Update();
  this->ROILabelMapNode->Modified();

  // Update the roiNode
  if(roiNode && !roiUpdateGuard)
    {
    roiUpdateGuard = true;
    MRMLUpdateROINodeFromROI();
    roiNode->Modified();
    roiUpdateGuard = false;
    }
  
  double *roiXYZ = roiNode->GetXYZ();      
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(roiXYZ[0], roiXYZ[1], roiXYZ[2]);

}


int vtkChangeTrackerROIStep::ROIMapShow() {
  // -----
  // Initialize
  if (!this->ROICheck()) {
    vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
                                     this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                     "Change Tracker", 
                                     "Please define VOI correctly before pressing button", 
                                     vtkKWMessageDialog::ErrorIcon);
    return 0;
  }

  vtkMRMLChangeTrackerNode* Node      =  this->GetGUI()->GetNode();
  if (!Node) return 0;
  vtkMRMLScene* mrmlScene           =  Node->GetScene();
  vtkMRMLNode* mrmlFristScanRefNode =  mrmlScene->GetNodeByID(Node->GetScan1_Ref());
  vtkMRMLVolumeNode* volumeNode     =  vtkMRMLVolumeNode::SafeDownCast(mrmlFristScanRefNode);
  if (!volumeNode) return 0;
  int* dimensions = volumeNode->GetImageData()->GetDimensions();

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove(); 

  // -----
  // Define LabelMap 
  this->ROILabelMap =  vtkImageRectangularSource::New();
  this->ROILabelMap->SetWholeExtent(0,dimensions[0] -1,0,dimensions[1] -1, 0,dimensions[2] -1); 
  this->ROILabelMap->SetOutputScalarTypeToShort();
  this->ROILabelMap->SetInsideGraySlopeFlag(0); 
  this->ROILabelMap->SetInValue(17);
  this->ROILabelMap->SetOutValue(0);
  this->ROILabelMap->Update();

  // Show map in Slicer 3 
  //  set scene [[$this GetLogic] GetMRMLScene]
  //  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
//  vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();

  vtkSlicerVolumesGUI  *volumesGUI    = vtkSlicerVolumesGUI::SafeDownCast(application->GetModuleGUIByName("Volumes")); 
  vtkSlicerVolumesLogic *volumesLogic = volumesGUI->GetLogic();
  // set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]
  this->ROILabelMapNode = volumesLogic->CreateLabelVolume(mrmlScene,volumeNode, "TG_ROI");

  vtkSmartPointer<vtkSlicerColorLogic> colorLogic =
    vtkSmartPointer<vtkSlicerColorLogic>::New();
  this->ROILabelMapNode->GetDisplayNode()->SetAndObserveColorNodeID
    (colorLogic->GetDefaultColorTableNodeID(vtkMRMLColorTableNode::Labels));

  this->ROILabelMapNode->SetAndObserveImageData(this->ROILabelMap->GetOutput());

  // Now show in foreground 
  //  make the source node the active background, and the label node the active label
  // set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  //$selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  //$selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID] 
  //  applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(volumeNode->GetID());
 
  // Reset to original slice location 
  double oldSliceSetting[3];
  oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceOffset());
  oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceOffset());
  oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceOffset());

  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());

  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);

  // Reset to original slice location 
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->SetSliceOffset(oldSliceSetting[1]);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->SetSliceOffset(oldSliceSetting[2]);

  this->ROIMapUpdate();

  return 1;
}

void vtkChangeTrackerROIStep::ROIMapRemove() {
  
  if (this->ROILabelMapNode && this->GetGUI()) { 
    this->GetGUI()->GetMRMLScene()->RemoveNode(this->ROILabelMapNode);
  }
  this->ROILabelMapNode = NULL;

  if (this->ROILabelMap) { 
    this->ROILabelMap->Delete();
    this->ROILabelMap = NULL;
  }

  // Needs to be done otherwise when going backwards field is not correctly defined   
  if (this->ButtonsShow && this->ButtonsShow->IsCreated())  {
    this->ButtonsShow->SetText("Show render");
  }
}

void vtkChangeTrackerROIStep::RetrieveInteractorCoordinates(vtkSlicerSliceGUI *sliceGUI, 
                                                               vtkRenderWindowInteractor *rwi,
                                                               int coordsIJK[3], double coordsRAS[3]) 
{
  coordsIJK[0] = coordsIJK[1] = coordsIJK[2] = -1;
  coordsRAS[0] = coordsRAS[1] = coordsRAS[2] = -1.;
  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  if (!Node) {
    cout << "ERROR: vtkChangeTrackerROIStep::RetrieveInteractorCoordinates: No Node" << endl;
    return;
  } 

  if (!Node->GetScan1_Ref()) {
    cout << "ERROR: vtkChangeTrackerROIStep::RetrieveInteractorCoordinates: No First Volume Defined" << endl;
    return;
  }
  vtkMRMLNode* mrmlNode =   Node->GetScene()->GetNodeByID(Node->GetScan1_Ref());
  vtkMRMLVolumeNode* volumeNode =  vtkMRMLVolumeNode::SafeDownCast(mrmlNode);

  if (!volumeNode)
    {
      cout << "ERROR: vtkChangeTrackerROIStep::RetrieveInteractorCoordinates: No Scan1_Ref" << endl;
      return;
    }

  // --------------------------------------------------------------
  // Compute RAS coordinates
  int point[2];
  rwi->GetLastEventPosition(point);
  double inPt[4] = {point[0], point[1], 0, 1};
  double rasPt[4];
  vtkMatrix4x4 *matrix = sliceGUI->GetLogic()->GetSliceNode()->GetXYToRAS();
  matrix->MultiplyPoint(inPt, rasPt); 
  matrix = NULL;

  coordsRAS[0] = rasPt[0];
  coordsRAS[1] = rasPt[1];
  coordsRAS[2] = rasPt[2];

  // --------------------------------------------------------------
  // Compute IJK coordinates
  double ijkPt[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPt, ijkPt);
  rasToijk->Delete();

  // --------------------------------------------------------------
  // Check validity of coordinates
  int* dimensions = volumeNode->GetImageData()->GetDimensions();
  for (int i = 0 ; i < 3 ; i++) {
    if (ijkPt[i] < 0 ) ijkPt[i] = 0;
    else if (ijkPt[i] >=  dimensions[i] ) ijkPt[i] = dimensions[i] -1;    
  }
  //coordsIJK[0] = int(0.5+(ijkPt[0]));  
  //coordsIJK[1] = int(0.5+(ijkPt[1])); 
  //coordsIJK[2] = int(0.5+(ijkPt[2])); 
  coordsIJK[0] = int(ijkPt[0]);  
  coordsIJK[1] = int(ijkPt[1]); 
  coordsIJK[2] = int(ijkPt[2]); 

  //cout << "Sample:  " << rasPt[0] << " " <<  rasPt[1] << " " << rasPt[2] << " " << rasPt[3] << endl;
  //cout << "Coord: " << coords[0] << " " << coords[1] << " " << coords[2] << " " << coords[3] << endl;
  //cout << "Dimen: " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " " <<  endl;

}

void vtkChangeTrackerROIStep::ProcessGUIEvents(vtkObject *caller, unsigned long event,
                                               void *vtkNotUsed(callData)) {
  
  if (event == vtkKWPushButton::InvokedEvent) {
    vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
    if (this->ButtonsShow && (button == this->ButtonsShow)) 
    { 
      if (this->ROILabelMapNode) {
        this->ButtonsShow->SetText("Show render");
        this->ROIMapRemove();
//        roiNode->SetVisibility(0);
        ResetROIRender();
        this->ROIHideFlag = 1;
        this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
      } else { 
        if (this->ROIMapShow()) { 
//          roiNode->SetVisibility(1);
          UpdateROIRender();
          this->ButtonsShow->SetText("Hide render");
          this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());
        }
// FIXME: when feature complete
//        if (roiNode)  
//          roiNode->SetVisibility(1);
      }
    }
    if (this->ButtonsReset && (button == this->ButtonsReset)) 
    { 
      if (this->ROILabelMapNode) {
        this->ButtonsShow->SetText("Show render");
        this->ROIMapRemove();
        roiNode->SetVisibility(0);
        this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
        ResetROIRender();
      }
      this->ROIReset();
      this->MRMLUpdateROIFromROINode();
    }
    return;
  }

  vtkSlicerInteractorStyle *s = vtkSlicerInteractorStyle::SafeDownCast(caller);
  if (s)
  {
    // Retrieve Coordinates and update ROI
    int index = 0; 
    vtkSlicerSliceGUI *sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red");

    vtkRenderWindowInteractor *rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    while (index < 2 && (s != rwi->GetInteractorStyle())) {
        index ++;
        if (index == 1) {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow");
        } else {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green");
        }
        rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    }
    int ijkCoords[3];
    double rasCoords[3];
    this->RetrieveInteractorCoordinates(sliceGUI, rwi, ijkCoords, rasCoords);
    
    if(event == vtkCommand::LeftButtonPressEvent)
      {
      this->ROIUpdateWithNewSample(rasCoords);
      }
    else if(event == vtkCommand::RightButtonPressEvent)
      {
      this->roiNode->SetXYZ(rasCoords);
      }

    if (!this->ROILabelMapNode && !this->ROIHideFlag && this->ROICheck()) {
      if (this->ROIMapShow()) 
        {
        MRMLUpdateROINodeFromROI();
        roiNode->SetVisibility(1);
        this->ButtonsShow->SetText("Hide render");
        }
    }
    UpdateROIRender();
  }  
  // Define SHOW Button 
}


void vtkChangeTrackerROIStep::ProcessMRMLEvents(vtkObject *caller, unsigned long event,
                                                void *vtkNotUsed(callData))
{
//  if(event == vtkCommand::ModifiedEvent){
    vtkMRMLROINode *roiCaller = vtkMRMLROINode::SafeDownCast(caller);
    if(roiCaller && roiCaller == roiNode && event == vtkCommand::ModifiedEvent && !roiUpdateGuard)
      {
      
      roiUpdateGuard = true;
      double *roiXYZ = roiNode->GetXYZ();
      double *roiRadius = roiNode->GetRadiusXYZ();
      double newROIXYZ[3], newROIRadius[3];
      newROIXYZ[0] = ceil(roiXYZ[0]);
      newROIXYZ[1] = ceil(roiXYZ[1]);
      newROIXYZ[2] = ceil(roiXYZ[2]);
      newROIRadius[0] = ceil(roiRadius[0]);
      newROIRadius[1] = ceil(roiRadius[1]);
      newROIRadius[2] = ceil(roiRadius[2]);
      std::cerr << "ROI XYZ: " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << std::endl;
      roiNode->SetXYZ(newROIXYZ);
      roiNode->SetRadiusXYZ(newROIRadius);
      MRMLUpdateROIFromROINode();
      this->ROIMapUpdate();
      if(this->Render_Filter->GetSize())
        this->UpdateROIRender();
      roiUpdateGuard = false;

      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
      app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(newROIXYZ[0], newROIXYZ[1], newROIXYZ[2]);
//      cerr << "Resetting focal point to " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << endl;
      }
}

// Propagate ROI changes in ChangeTracker MRML to ROINode MRML
void vtkChangeTrackerROIStep::MRMLUpdateROIFromROINode()
{
  vtkMRMLChangeTrackerNode* ctNode = this->GetGUI()->GetNode();
  vtkMRMLVolumeNode *volumeNode = 
    vtkMRMLVolumeNode::SafeDownCast(ctNode->GetScene()->GetNodeByID(ctNode->GetScan1_Ref()));
  if(!volumeNode)
    return;

  // update roi to correspond to ROI widget
  double *roiXYZ = roiNode->GetXYZ();
  double *roiRadiusXYZ = roiNode->GetRadiusXYZ();

  double bbox0ras[4], bbox1ras[4];
  double bbox0ijk[4], bbox1ijk[4];

  // ROI bounding box in RAS coordinates
  bbox0ras[0] = roiXYZ[0]-roiRadiusXYZ[0];
  bbox0ras[1] = roiXYZ[1]-roiRadiusXYZ[1];
  bbox0ras[2] = roiXYZ[2]-roiRadiusXYZ[2];
  bbox0ras[3] = 1.;
  bbox1ras[0] = roiXYZ[0]+roiRadiusXYZ[0];
  bbox1ras[1] = roiXYZ[1]+roiRadiusXYZ[1];
  bbox1ras[2] = roiXYZ[2]+roiRadiusXYZ[2];
  bbox1ras[3] = 1.;

  vtkMatrix4x4 *rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(bbox0ras,bbox0ijk);
  rasToijk->MultiplyPoint(bbox1ras,bbox1ijk);
  rasToijk->Delete();
  for(int i=0;i<3;i++)
    {
    double tmp;
    if(bbox0ijk[i]>bbox1ijk[i])
      {
      tmp = bbox0ijk[i];
      bbox0ijk[i] = bbox1ijk[i];
      bbox1ijk[i] = tmp;
      }
    }
  ctNode->SetROIMin(0, (int)bbox0ijk[0]);
  ctNode->SetROIMax(0, (int)bbox1ijk[0]);
  ctNode->SetROIMin(1, (int)bbox0ijk[1]);
  ctNode->SetROIMax(1, (int)bbox1ijk[1]);
  ctNode->SetROIMin(2, (int)bbox0ijk[2]);
  ctNode->SetROIMax(2, (int)bbox1ijk[2]);
  
#ifdef SHOWROIIJK
  ROIX->SetRange(ctNode->GetROIMin(0), ctNode->GetROIMax(0));
  ROIY->SetRange(ctNode->GetROIMin(1), ctNode->GetROIMax(1));
  ROIZ->SetRange(ctNode->GetROIMin(2), ctNode->GetROIMax(2));
#endif // SHOWROIIJK
}

// Propagate changes in ROINode MRML to ChangeTracker ROI MRML
void vtkChangeTrackerROIStep::MRMLUpdateROINodeFromROI()
{
  double ROIMinIJK[4], ROIMaxIJK[4], ROIMinRAS[4], ROIMaxRAS[4];
  double radius[3], center[3];

  vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
  vtkMRMLVolumeNode *volumeNode = 
    vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));

  if(!volumeNode || !Node)
    return;

  ROIMinIJK[0] = Node->GetROIMin(0);
  ROIMinIJK[1] = Node->GetROIMin(1);
  ROIMinIJK[2] = Node->GetROIMin(2);
  ROIMinIJK[3] = 1.;
  ROIMaxIJK[0] = Node->GetROIMax(0);
  ROIMaxIJK[1] = Node->GetROIMax(1);
  ROIMaxIJK[2] = Node->GetROIMax(2);
  ROIMaxIJK[3] = 1.;

  vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToras);
  ijkToras->MultiplyPoint(ROIMinIJK,ROIMinRAS);
  ijkToras->MultiplyPoint(ROIMaxIJK,ROIMaxRAS);
  ijkToras->Delete();

  center[0] = (ROIMaxRAS[0]+ROIMinRAS[0])/2.;
  center[1] = (ROIMaxRAS[1]+ROIMinRAS[1])/2.;
  center[2] = (ROIMaxRAS[2]+ROIMinRAS[2])/2.;

  radius[0] = fabs(ROIMaxRAS[0]-ROIMinRAS[0])/2.;
  radius[1] = fabs(ROIMaxRAS[1]-ROIMinRAS[1])/2.;
  radius[2] = fabs(ROIMaxRAS[2]-ROIMinRAS[2])/2.;

  roiNode->SetXYZ(center[0], center[1], center[2]);
  roiNode->SetRadiusXYZ(radius[0], radius[1], radius[2]);
  roiNode->Modified();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::TransitionCallback() 
{
  // cout << "vtkChangeTrackerROIStep::TransitionCallback() Start" << endl; 
  double *roiXYZ, *roiRadius;
  roiXYZ = roiNode->GetXYZ();
  roiRadius = roiNode->GetRadiusXYZ();
//  std::cerr << "Final ROI center: " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << std::endl;
//  std::cerr << "Final ROI radius: " << roiRadius[0] << ", " << roiRadius[1] << ", " << roiRadius[2] << std::endl;
  if (this->ROICheck()) { 
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
    if (!Node) return;

    // update the node with the advanced settings
    vtkKWRadioButton *rc0 = this->ResamplingChoice->GetWidget()->GetWidget(0);
    vtkKWRadioButton *rc1 = this->ResamplingChoice->GetWidget()->GetWidget(1);
    vtkKWRadioButton *rc2 = this->ResamplingChoice->GetWidget()->GetWidget(2);
    if(rc0->GetSelectedState())
      {
      Node->SetResampleChoice(RESCHOICE_NONE);
      }
    else if(rc1->GetSelectedState())
      {
      Node->SetResampleChoice(RESCHOICE_LEGACY);
      }
    else if(rc2->GetSelectedState())
      {
      Node->SetResampleChoice(RESCHOICE_ISO);
      }
    Node->SetResampleConst(this->SpinResampleConst->GetWidget()->GetValue());

    // ----------------------------
    // Create SuperSampledVolume 
    vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
    vtkMRMLScalarVolumeNode *outputNode = this->GetGUI()->GetLogic()->CreateSuperSample(1);
    this->GetGUI()->GetLogic()->SaveVolume(application,outputNode); 

    if (outputNode) {
       // Prepare to update mrml node with results 
              
       // Delete old attached node first 
       this->GetGUI()->GetLogic()->DeleteSuperSample(1);

       // Update node 
       
       Node->SetScan1_SuperSampleRef(outputNode->GetID());
       //cout << "==============================" << endl;
       //cout << "vtkChangeTrackerROIStep::TransitionCallback " << Node->GetScan1_SuperSampleRef() << " " <<  Node->GetScan1_Ref() << endl;
       //cout << "==============================" << endl;

       // Remove blue ROI screen 
       this->ROIMapRemove();
       
       // remove the ROI widget
       if (roiNode)
         roiNode->SetVisibility(0);
       ResetROIRender();

//       std::cout << "Final ROI in IJK:  X " << Node->GetROIMin(0) << "," << Node->GetROIMax(0) 
//            << "                   Y " << Node->GetROIMin(1) << "," << Node->GetROIMax(1) 
//            << "                   Z " << Node->GetROIMin(2) << "," << Node->GetROIMax(2) << std::endl;


       this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
     } else {
       vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
                                        this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                        "Change Tracker", 
                                        "Could not proceed to next step - scan1 might have disappeared", 
                                        vtkKWMessageDialog::ErrorIcon); 
     }
     // ---------------------------------
   } else {     
     vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
                                      this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                      "Change Tracker", 
                                      "Please define VOI correctly before proceeding", 
                                      vtkKWMessageDialog::ErrorIcon);
   }
  
}


//----------------------------------------------------------------------------
void  vtkChangeTrackerROIStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveROISamplingGUIObservers();
}

//----------------------------------------------------------------------------
void vtkChangeTrackerROIStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkChangeTrackerROIStep::ROIIntensityMinMaxUpdate(vtkImageData* image, double &intensityMin, double &intensityMax)
{
  if(!this->ROICheck())
  {
    return;
  }

  vtkMRMLChangeTrackerNode* ctNode = this->GetGUI()->GetNode();
  if(
    ctNode->GetROIMax(0)-ctNode->GetROIMin(0)<=0 ||
    ctNode->GetROIMax(1)-ctNode->GetROIMin(1)<=0 ||
    ctNode->GetROIMax(2)-ctNode->GetROIMin(2)<=0)
    {
    return;
    }

  int ijk[3], ijkMin[3], ijkMax[3], ijkInc[3], i;
  
  for(i=0;i<3;i++)
    {
    ijkMax[i] = ctNode->GetROIMax(i);
    ijkMin[i] = ctNode->GetROIMin(i);
    
    ijkInc[i] = (ijkMax[i]-ijkMin[i])/10;
    if(ijkInc[i]<=0)
      ijkInc[i] = 1;
    }

  intensityMin = image->GetScalarComponentAsDouble(ijkMin[0],ijkMin[1],ijkMin[2],0);
  intensityMax = image->GetScalarComponentAsDouble(ijkMin[0],ijkMin[1],ijkMin[2],0);

  for(ijk[0]=ijkMin[0];ijk[0]<ijkMax[0];ijk[0]+=ijkInc[0])
    {
    for(ijk[1]=ijkMin[1];ijk[1]<ijkMax[1];ijk[1]+=ijkInc[1])
      {
      for(ijk[2]=ijkMin[2];ijk[2]<ijkMax[2];ijk[2]+=ijkInc[2])
        {
        double intensity =
          image->GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],0);
        if(intensityMin>intensity)
          intensityMin = intensity;
        if(intensityMax<intensity)
          intensityMax = intensity;
        }
      }
    }
}

void vtkChangeTrackerROIStep::InitROIRender()
{ 
  if(this->roiNode)
    {
    vtkMRMLChangeTrackerNode* Node = this->GetGUI()->GetNode();
    vtkMRMLVolumeNode* volumeNode =  
      vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
    if(volumeNode)
      {
      CreateRender(volumeNode, 0);
      }
    }
}

void vtkChangeTrackerROIStep::UpdateROIRender()
{
  vtkMRMLChangeTrackerNode* node = this->GetGUI()->GetNode();
  vtkMRMLVolumeNode *volumeNode = vtkMRMLVolumeNode::SafeDownCast(node->GetScene()->GetNodeByID(node->GetScan1_Ref()));
  if(volumeNode)
    {
    float color0[3] = { 0.8, 0.8, 0.0 };
    float color1[3] = { 0.8, 0.8, 0.0 };
    double intensityMin, intensityMax;
    
    double* imgRange  =   volumeNode->GetImageData()->GetPointData()->GetScalars()->GetRange();
    intensityMin = imgRange[0];
    intensityMax = imgRange[1];
    this->ROIIntensityMinMaxUpdate(volumeNode->GetImageData(), intensityMin, intensityMax);
    this->SetRender_BandPassFilter((intensityMax+intensityMin)*.4, intensityMax-1, color0, color1);

    if(this->Render_RayCast_Mapper)
      {
      this->Render_Mapper->SetCroppingRegionPlanes(node->GetROIMin(0), node->GetROIMax(0),
                                                   node->GetROIMin(1), node->GetROIMax(1),
                                                   node->GetROIMin(2), node->GetROIMax(2));
      this->Render_Mapper->CroppingOn();
      this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
      }

    if(this->Render_Mapper)
      {
      // !!!  Cropping region is defined in voxel coordinates !!!
      this->Render_Mapper->SetCroppingRegionPlanes(node->GetROIMin(0), node->GetROIMax(0),
                                                   node->GetROIMin(1), node->GetROIMax(1),
                                                   node->GetROIMin(2), node->GetROIMax(2));
        
      this->Render_Mapper->CroppingOn();
      this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
      }
    }
}

void vtkChangeTrackerROIStep::ResetROIRender()
{
  this->Render_Filter->RemoveAllPoints();
}

void vtkChangeTrackerROIStep::ResetROICenter(int *center)
{
  vtkMRMLChangeTrackerNode* Node      =  this->GetGUI()->GetNode();
  double pointRAS[4], pointIJK[4];
  vtkMRMLVolumeNode *volumeNode = 
    vtkMRMLVolumeNode::SafeDownCast(Node->GetScene()->GetNodeByID(Node->GetScan1_Ref()));
  if(!volumeNode)
    return;
  vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToras);
  pointIJK[0] = (double)center[0];
  pointIJK[1] = (double)center[1];
  pointIJK[2] = (double)center[2];
  pointIJK[3] = 1.;
  ijkToras->MultiplyPoint(pointIJK,pointRAS);
  ijkToras->Delete();

  roiNode->SetXYZ(pointRAS[0], pointRAS[1], pointRAS[2]);

  CenterRYGSliceViews(pointRAS[0], pointRAS[1], pointRAS[2]);
}

void vtkChangeTrackerROIStep::ROIMRMLCallback(vtkObject *caller,
                                              unsigned long event,
                                              void *clientData,
                                              void *vtkNotUsed(callData))
{
  vtkChangeTrackerROIStep *thisStep = reinterpret_cast<vtkChangeTrackerROIStep*>(clientData);

  vtkMRMLROINode *roiCaller = vtkMRMLROINode::SafeDownCast(caller);
  if(roiCaller && roiCaller == thisStep->roiNode && event == vtkCommand::ModifiedEvent && !thisStep->roiUpdateGuard)
    {
//    vtkMRMLChangeTrackerNode* node = thisStep->GetGUI()->GetNode();

    thisStep->roiUpdateGuard = true;

    thisStep->MRMLUpdateROIFromROINode();

//    thisStep->ROIX->SetRange(node->GetROIMin(0), node->GetROIMax(0));
//    thisStep->ROIY->SetRange(node->GetROIMin(1), node->GetROIMax(1));
//    thisStep->ROIZ->SetRange(node->GetROIMin(2), node->GetROIMax(2));

    thisStep->ROIMapUpdate();
    if(thisStep->Render_Filter->GetSize())
      thisStep->UpdateROIRender();
    thisStep->roiUpdateGuard = false;

    double *roiXYZ = thisStep->roiNode->GetXYZ();
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(thisStep->GetGUI()->GetApplication());
    app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(roiXYZ[0], roiXYZ[1], roiXYZ[2]);
    //      cerr << "Resetting focal point to " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << endl;
    }
}

