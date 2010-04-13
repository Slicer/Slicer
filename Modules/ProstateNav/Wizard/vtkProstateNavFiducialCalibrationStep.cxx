#include "vtkProstateNavFiducialCalibrationStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMath.h"
#include "vtkLineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntrySet.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "itkMetaDataObject.h"
#include "vtkImageChangeInformation.h"


#include "vtkMRMLTransRectalProstateRobotNode.h"

#include <vtksys/ios/sstream>

static const char MARKER_LABEL_NAMES[CALIB_MARKER_COUNT][9]={ "Marker 1", "Marker 2", "Marker 3", "Marker 4"};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavFiducialCalibrationStep);
vtkCxxRevisionMacro(vtkProstateNavFiducialCalibrationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavFiducialCalibrationStep::vtkProstateNavFiducialCalibrationStep()
{
  this->SetTitle("Calibration");
  this->SetDescription("Load calibration image and click on the markers.");

  this->CalibrationPointListNode=vtkSmartPointer<vtkMRMLFiducialListNode>::New();
  this->CalibrationPointListNode->SetName("CalibrationMarkers");

  this->LoadVolumeDialogFrame=vtkSmartPointer<vtkKWFrame>::New();
  this->LoadCalibrationVolumeButton=vtkSmartPointer<vtkKWPushButton>::New();
  this->VolumeSelectorWidget=vtkSmartPointer<vtkSlicerNodeSelectorWidget>::New();
  this->ResetCalibrationButton=vtkSmartPointer<vtkKWPushButton>::New();
  this->ResegmentButton=vtkSmartPointer<vtkKWPushButton>::New();
  this->EditMarkerPositionButton=vtkSmartPointer<vtkKWCheckButton>::New();  
  this->LoadResetFrame=vtkSmartPointer<vtkKWFrame>::New();
  this->ExportImportFrame=vtkSmartPointer<vtkKWFrame>::New();
  this->FiducialPropertiesFrame=vtkSmartPointer<vtkKWFrameWithLabel>::New();
  this->FiducialSizeLabel=vtkSmartPointer<vtkKWLabel>::New();
  this->FiducialWidthSpinBox=vtkSmartPointer<vtkKWSpinBoxWithLabel>::New();
  this->FiducialHeightSpinBox=vtkSmartPointer<vtkKWSpinBoxWithLabel>::New();
  this->FiducialDepthSpinBox=vtkSmartPointer<vtkKWSpinBoxWithLabel>::New();  
  for (unsigned int i = 0; i < CALIB_MARKER_COUNT; i++)
    {
    this->FiducialThresholdScale[i]=vtkSmartPointer<vtkKWScaleWithEntry>::New();
    this->JumpToFiducialButton[i]=vtkSmartPointer<vtkKWPushButton>::New();
    }
  this->AutomaticCenterpointAdjustmentCheckButton=vtkSmartPointer<vtkKWCheckButton>::New();
  this->RadiusSpinBox=vtkSmartPointer<vtkKWSpinBoxWithLabel>::New();
  this->RadiusCheckButton=vtkSmartPointer<vtkKWCheckButton>::New();
  this->InitialAngleSpinBox=vtkSmartPointer<vtkKWSpinBoxWithLabel>::New();
  this->SegmentationResultsFrame=vtkSmartPointer<vtkKWFrameWithLabel>::New();
  this->CalibrationResultsBox=vtkSmartPointer<vtkKWTextWithScrollbars>::New();

  this->ProcessingCallback = false;

  this->ObservedRobot=NULL;
  
  this->EnableAutomaticCenterpointAdjustment=true;
}

//----------------------------------------------------------------------------
vtkProstateNavFiducialCalibrationStep::~vtkProstateNavFiducialCalibrationStep()
{
  this->CalibrationPointListNode->SetAndObserveTransformNodeID(NULL);
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowLoadResetControls()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();
  
  if (!this->LoadResetFrame->IsCreated())
    {
    this->LoadResetFrame->SetParent(parent);
    this->LoadResetFrame->Create();     
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->LoadResetFrame->GetWidgetName());

  if (!this->InitialAngleSpinBox->IsCreated())
    {
    this->InitialAngleSpinBox->SetParent(this->LoadResetFrame);
    this->InitialAngleSpinBox->Create();
    this->InitialAngleSpinBox->GetWidget()->SetWidth(11);
    this->InitialAngleSpinBox->GetWidget()->SetRange(-180,180);
    this->InitialAngleSpinBox->GetWidget()->SetIncrement(1);
    this->InitialAngleSpinBox->GetWidget()->SetValue(0);
    this->InitialAngleSpinBox->SetLabelText("Initial rotation angle");
    //this->InitialAngleSpinBox->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, this->WizardGUICallbackCommand);
    }
  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", this->InitialAngleSpinBox->GetWidgetName() );
  
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowLoadVolumeControls()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();
   
  if (!this->LoadVolumeDialogFrame->IsCreated())
    {
    this->LoadVolumeDialogFrame->SetParent(parent);
    this->LoadVolumeDialogFrame->Create();     
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->LoadVolumeDialogFrame->GetWidgetName());
  
  if (!this->LoadCalibrationVolumeButton->IsCreated())
    {
    this->LoadCalibrationVolumeButton->SetParent(this->LoadVolumeDialogFrame);
    this->LoadCalibrationVolumeButton->Create();
    this->LoadCalibrationVolumeButton->SetBorderWidth(2);
    this->LoadCalibrationVolumeButton->SetReliefToRaised();       
    this->LoadCalibrationVolumeButton->SetHighlightThickness(2);
    this->LoadCalibrationVolumeButton->SetBackgroundColor(0.85,0.85,0.85);
    this->LoadCalibrationVolumeButton->SetActiveBackgroundColor(1,1,1);        
    this->LoadCalibrationVolumeButton->SetText( "Load volume");
    this->LoadCalibrationVolumeButton->SetBalloonHelpString("Click to load a volume. Need to additionally select the volume to make it the current calibration volume.");
    }
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->LoadCalibrationVolumeButton->GetWidgetName());

  if (!this->VolumeSelectorWidget->IsCreated())
    {
    this->VolumeSelectorWidget->SetParent(this->LoadVolumeDialogFrame);
    this->VolumeSelectorWidget->Create();
    this->VolumeSelectorWidget->SetBorderWidth(2);  
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetLogic()->GetApplicationLogic()->GetMRMLScene());
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->VolumeSelectorWidget->SetLabelText( "Calibration Volume: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("Select the calibration volume from the current scene.");    
    }
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->VolumeSelectorWidget->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowFiducialSegmentParamsControls()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();
  
  if (!this->FiducialPropertiesFrame->IsCreated())
    {
    this->FiducialPropertiesFrame->SetParent(parent);
    this->FiducialPropertiesFrame->Create();
    this->FiducialPropertiesFrame->SetLabelText("Fiducial properties");
    this->FiducialPropertiesFrame->CollapseFrame();    
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FiducialPropertiesFrame->GetWidgetName());

  if (!this->FiducialSizeLabel->IsCreated())
    {
    this->FiducialSizeLabel->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->FiducialSizeLabel->Create();
    this->FiducialSizeLabel->SetText("Fiducial size:");
    }
  this->Script("grid %s -row 0 -column 0 -sticky ew -padx 2 -pady 2", this->FiducialSizeLabel->GetWidgetName());

  if (!this->FiducialWidthSpinBox->IsCreated())
    {
    this->FiducialWidthSpinBox->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->FiducialWidthSpinBox->Create();
    this->FiducialWidthSpinBox->GetWidget()->SetWidth(11);
    this->FiducialWidthSpinBox->GetWidget()->SetRange(0,100);
    this->FiducialWidthSpinBox->GetWidget()->SetIncrement(1);
    this->FiducialWidthSpinBox->GetWidget()->SetValue(8);
    this->FiducialWidthSpinBox->SetLabelText("W");
    this->FiducialWidthSpinBox->SetBalloonHelpString("fiducial width in mm");
    //this->FiducialWidthSpinBox->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, this->WizardGUICallbackCommand);
    }
  if (!this->FiducialHeightSpinBox->IsCreated())
    {
    this->FiducialHeightSpinBox->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->FiducialHeightSpinBox->Create();
    this->FiducialHeightSpinBox->GetWidget()->SetWidth(11);
    this->FiducialHeightSpinBox->GetWidget()->SetRange(0,100);
    this->FiducialHeightSpinBox->GetWidget()->SetIncrement(1);
    this->FiducialHeightSpinBox->GetWidget()->SetValue(5);
    this->FiducialHeightSpinBox->SetLabelText("H");
    this->FiducialHeightSpinBox->SetBalloonHelpString("fiducial height in mm");
    //this->FiducialHeightSpinBox->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, this->WizardGUICallbackCommand);
    }
  if (!this->FiducialDepthSpinBox->IsCreated())
    {
    this->FiducialDepthSpinBox->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->FiducialDepthSpinBox->Create();
    this->FiducialDepthSpinBox->GetWidget()->SetWidth(11);
    this->FiducialDepthSpinBox->GetWidget()->SetRange(0,100);
    this->FiducialDepthSpinBox->GetWidget()->SetIncrement(1);
    this->FiducialDepthSpinBox->GetWidget()->SetValue(5);
    this->FiducialDepthSpinBox->SetLabelText("D");
    this->FiducialDepthSpinBox->SetBalloonHelpString("fiducial depth in mm");
    //this->FiducialDepthSpinBox->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, this->WizardGUICallbackCommand);
    }
  /*this->Script("pack %s %s %s -side top -anchor w -padx 2 -pady 2", 
               this->FiducialWidthSpinBox->GetWidgetName(),
               this->FiducialHeightSpinBox->GetWidgetName(),
               this->FiducialDepthSpinBox->GetWidgetName());
               */

  this->Script("grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 2", this->FiducialWidthSpinBox->GetWidgetName());
  this->Script("grid %s -row 0 -column 2 -sticky ew -padx 2 -pady 2", this->FiducialHeightSpinBox->GetWidgetName());
  this->Script("grid %s -row 0 -column 3 -sticky ew -padx 2 -pady 2", this->FiducialDepthSpinBox->GetWidgetName());

  if (!this->RadiusSpinBox->IsCreated())
    {
    this->RadiusSpinBox->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->RadiusSpinBox->Create();
    this->RadiusSpinBox->GetWidget()->SetWidth(11);
    this->RadiusSpinBox->GetWidget()->SetRange(1,10);
    this->RadiusSpinBox->GetWidget()->SetIncrement(0.1);
    this->RadiusSpinBox->GetWidget()->SetValue(3.5);
    this->RadiusSpinBox->SetLabelText("Fiducial radius (mm)");
    //this->RadiusSpinBox->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent, this->WizardGUICallbackCommand);
    }
  //this->Script("pack %s -side top -anchor w -padx 2 -pady 2", this->RadiusSpinBox->GetWidgetName());
  this->Script("grid %s -row 1 -column 0 -columnspan 2 -padx 2 -pady 2", this->RadiusSpinBox->GetWidgetName());

  for (unsigned int i = 0; i < CALIB_MARKER_COUNT; i++)
    {
      {
        std::ostrstream ospack;
        ospack << "grid";
        if (!this->JumpToFiducialButton[i]->IsCreated())
        {  
          this->JumpToFiducialButton[i]->SetParent(this->FiducialPropertiesFrame->GetFrame());
          this->JumpToFiducialButton[i]->Create();
          std::ostrstream os;
          os << "Marker "<< i+1 << std::ends;    
          this->JumpToFiducialButton[i]->SetText(os.str());
          os.rdbuf()->freeze();
        }
        ospack << " " << this->JumpToFiducialButton[i]->GetWidgetName();
        ospack << " -row "<<2+i<<" -column 0 -padx 2 -pady 2" << std::ends;
        this->Script(ospack.str());
        ospack.rdbuf()->freeze();
        ospack.clear();
      }

      {
        std::ostrstream ospack;
        ospack << "grid";
        if (!this->FiducialThresholdScale[i]->IsCreated())
        {  
          this->FiducialThresholdScale[i]->SetParent(this->FiducialPropertiesFrame->GetFrame());
          this->FiducialThresholdScale[i]->Create();
          std::ostrstream os;
          os << "Threshold "<< i+1 << std::ends;    
          this->FiducialThresholdScale[i]->SetLabelText(os.str());
          os.rdbuf()->freeze();
          this->FiducialThresholdScale[i]->SetWidth(3);
          this->FiducialThresholdScale[i]->SetLength(200);
          this->FiducialThresholdScale[i]->SetRange(0,100);
          this->FiducialThresholdScale[i]->SetResolution(0.5);
          this->FiducialThresholdScale[i]->SetValue(9);
          //this->FiducialThresholdScale[i]->AddObserver(vtkKWScale::ScaleValueChangingEvent, this->WizardGUICallbackCommand);            
        }
        ospack << " " << this->FiducialThresholdScale[i]->GetWidgetName();
        ospack << " -row "<<2+i<<" -column 1 -columnspan 3 -padx 2 -pady 2" << std::ends;
        this->Script(ospack.str());
        ospack.rdbuf()->freeze();
        ospack.clear();
      }
    }

  // create the Calibrate button
  if (!this->AutomaticCenterpointAdjustmentCheckButton->IsCreated())
    {
    this->AutomaticCenterpointAdjustmentCheckButton->SetParent(this->FiducialPropertiesFrame->GetFrame());
    this->AutomaticCenterpointAdjustmentCheckButton->Create();
    this->AutomaticCenterpointAdjustmentCheckButton->SetText("Enable automatic marker centerpoint detection");
    this->AutomaticCenterpointAdjustmentCheckButton->SetBalloonHelpString("Click to set calibration marker positions on the images");
    this->AutomaticCenterpointAdjustmentCheckButton->SetEnabled(1);
    }    
   this->Script("grid %s -row 6 -column 0 -columnspan 3 -sticky e -padx 2 -pady 4", this->AutomaticCenterpointAdjustmentCheckButton->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowFiducialSegmentationResultsControls()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();
   
  // Create the frame
  if (!this->SegmentationResultsFrame->IsCreated())
    {
    this->SegmentationResultsFrame->SetParent(parent);
    this->SegmentationResultsFrame->Create();
    this->SegmentationResultsFrame->SetLabelText("Segmentation/registration results");
    this->SegmentationResultsFrame->SetBalloonHelpString("Results of segmentation and robot registration");
    this->SegmentationResultsFrame->AllowFrameToCollapseOff();
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
                    this->SegmentationResultsFrame->GetWidgetName());

  if(!this->CalibrationResultsBox->IsCreated())
    {
    this->CalibrationResultsBox->SetParent(this->SegmentationResultsFrame->GetFrame());
    this->CalibrationResultsBox->Create();
    this->CalibrationResultsBox->SetHorizontalScrollbarVisibility(0);
    this->CalibrationResultsBox->SetVerticalScrollbarVisibility( 1) ;
    this->CalibrationResultsBox->GetWidget()->SetText("");      
    this->CalibrationResultsBox->GetWidget()->SetBackgroundColor(0.7, 0.7, 0.95);
    this->CalibrationResultsBox->SetHeight(6);
    this->CalibrationResultsBox->GetWidget()->SetWrapToWord();
    this->CalibrationResultsBox->GetWidget()->ReadOnlyOn();
    this->CalibrationResultsBox->SetBorderWidth(2);
    this->CalibrationResultsBox->SetReliefToGroove();
    this->CalibrationResultsBox->GetWidget()->SetFont("times 12 bold");
    }
  this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 6", this->CalibrationResultsBox->GetWidgetName());  

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowExportImportControls()
{
   vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();
  
  // Create the frame
  if (!this->ExportImportFrame->IsCreated())
    {
    this->ExportImportFrame->SetParent(parent);
    this->ExportImportFrame->Create();
    }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->ExportImportFrame->GetWidgetName());

  // create the Calibrate button
  if (!this->EditMarkerPositionButton->IsCreated())
    {
    this->EditMarkerPositionButton->SetParent(this->ExportImportFrame);
    this->EditMarkerPositionButton->Create();
    this->EditMarkerPositionButton->SetText("Edit marker positions");
    this->EditMarkerPositionButton->SetBalloonHelpString("Click to set calibration marker positions on the images");
    this->EditMarkerPositionButton->SetEnabled(1);
    }    
   this->Script("grid %s -row 0 -column 0 -sticky ew -padx 2 -pady 4", this->EditMarkerPositionButton->GetWidgetName());

   // create the resegment button
  if (!this->ResegmentButton->IsCreated())
    {
    this->ResegmentButton->SetParent(this->ExportImportFrame);
    this->ResegmentButton->Create();
    this->ResegmentButton->SetText("Re-segment");
    this->ResegmentButton->SetBorderWidth(2);
    this->ResegmentButton->SetReliefToRaised();      
    this->ResegmentButton->SetHighlightThickness(2);
    this->ResegmentButton->SetBackgroundColor(0.85,0.85,0.85);
    this->ResegmentButton->SetActiveBackgroundColor(1,1,1);      
    this->ResegmentButton->SetBalloonHelpString("Click to re-segment the fiducials, with new parameters, but old fiducial location guesses. If you wish to give fiducial guesses again, press 'Reset calibration' button on top of the panel");
    //this->ResegmentButton->SetImageToPredefinedIcon(vtkKWIcon::IconTrashcan);
    this->ResegmentButton->SetEnabled(1);
    }    
  //this->Script("pack %s -side left -anchor nw -padx 2 -pady 4", this->ResegmentButton->GetWidgetName());
  this->Script("grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 4", this->ResegmentButton->GetWidgetName());

  // create the reset calib button
  if(!this->ResetCalibrationButton->IsCreated())
    {
    this->ResetCalibrationButton->SetParent(this->ExportImportFrame);
    this->ResetCalibrationButton->Create();
    this->ResetCalibrationButton->SetText("Reset calibration");
    this->ResetCalibrationButton->SetBorderWidth(2);
    this->ResetCalibrationButton->SetReliefToRaised();      
    this->ResetCalibrationButton->SetHighlightThickness(2);
    this->ResetCalibrationButton->SetBackgroundColor(0.85,0.85,0.85);
    this->ResetCalibrationButton->SetActiveBackgroundColor(1,1,1);      
    //this->ResetCalibrationButton->SetImageToPredefinedIcon(vtkKWIcon::IconTrashcan);
    this->ResetCalibrationButton->SetBalloonHelpString("Click to re-start the calibration, with new fiducial location guesses");    
    }
    
  //this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->ResetCalibrationButton->GetWidgetName());
  this->Script("grid %s -row 0 -column 2 -sticky ew -padx 2 -pady 4", this->ResetCalibrationButton->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  this->ShowLoadVolumeControls();
  this->ShowLoadResetControls();  
  this->ShowFiducialSegmentParamsControls();
  this->ShowFiducialSegmentationResultsControls();
  this->ShowExportImportControls();

  this->ShowAxesIn3DView(true);
  this->ShowMarkerVolumesIn3DView(true);

  vtkMRMLProstateNavManagerNode *manager= this->GetGUI()->GetProstateNavManager();
  if(manager!=NULL)
  {
    const char* calVolNodeID = manager->GetCalibrationVolumeNodeID();
    vtkMRMLScalarVolumeNode *calVolNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(calVolNodeID));
    if ( calVolNode )
    {
      this->VolumeSelectorWidget->UpdateMenu();
      this->VolumeSelectorWidget->SetSelected( calVolNode );
    }
  }

  vtkMRMLScene* scene=NULL;
  if (this->GetLogic()
    && this->GetLogic()->GetApplicationLogic()
    && this->GetLogic()->GetApplicationLogic()->GetMRMLScene())
  {
    scene=this->GetLogic()->GetApplicationLogic()->GetMRMLScene();
  }
  if (scene!=NULL)
  {
    if (this->CalibrationPointListNode->GetScene()!=scene)
    {
      scene->AddNode(this->CalibrationPointListNode);
    }
    vtkMRMLScalarVolumeNode *calVolNode=vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(manager->GetCalibrationVolumeNodeID()));    
    if (calVolNode!=NULL)
    {
      this->CalibrationPointListNode->SetAndObserveTransformNodeID(calVolNode->GetTransformNodeID());
    }
  }
   
  // update calibration marker positions from fiducials (needed for testing&debugging)
  vtkMRMLTransRectalProstateRobotNode* robot= GetRobot();
  if(robot==NULL)
  {
    vtkErrorMacro("UpdateCalibration: node is invalid");
  }
  else
  {
    unsigned int fidCount=this->CalibrationPointListNode->GetNumberOfFiducials();
    for (unsigned int i=0; i<CALIB_MARKER_COUNT && i<fidCount; i++)
    {
      float* rasPoint=this->CalibrationPointListNode->GetNthFiducialXYZ(i);
      double rasPointDouble[3]={rasPoint[0], rasPoint[1], rasPoint[2]};
      robot->SetCalibrationMarker(i, rasPointDouble);
    }
    if (this->CalibrationPointListNode->GetNumberOfFiducials()>=(int)CALIB_MARKER_COUNT)
    {
      //Resegment(); no need to resegment, as the node modified event will trigger UpdateCalibration();
    }
  }
  EnableMarkerPositionEdit(this->EditMarkerPositionButton->GetSelectedState() == 1);

  AddGUIObservers();
  AddMRMLObservers();
}
//-------------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();

  vtkMRMLTransRectalProstateRobotNode* robot= GetRobot();
  if(robot!=NULL)
  {
    robot->SetModelAxesVisible(false);
  }

  TearDownGUI();
}

//----------------------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::TearDownGUI()
{
  RemoveMRMLObservers();
  RemoveGUIObservers();
  
  vtkMRMLScene* scene=NULL;
  if (this->GetLogic())
  {
    if (this->GetLogic()->GetApplicationLogic())
    {
      if (this->GetLogic()->GetApplicationLogic()->GetMRMLScene())
      {
        scene=this->GetLogic()->GetApplicationLogic()->GetMRMLScene();
      }
    }
  }
  if (scene!=NULL)
  {
    scene->RemoveNode(this->CalibrationPointListNode);
    this->CalibrationPointListNode->SetScene(NULL);    
  }  
  this->CalibrationPointListNode->SetAndObserveTransformNodeID(NULL);
}


//----------------------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::PopulateCalibrationResults()
{
  std::ostrstream os;  
  
  vtkMRMLTransRectalProstateRobotNode* robot=GetRobot();
  TRProstateBiopsyCalibrationData calibData;
  if (robot!=NULL)
  {    
    calibData=robot->GetCalibrationData();
  }

  if (robot!=NULL && calibData.CalibrationValid)
  {
    os << "Calibration results:"<<std::endl;

    os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);
    os << "  Axes angle: "<<calibData.AxesAngleDegrees<<" deg"<<std::endl;  
    os << "  Axes distance: "<<calibData.AxesDistance<<" mm"<<std::endl;  
    os << "  Initial rotation angle: "<<calibData.RobotRegistrationAngleDegrees<<" deg"<<std::endl;  
    os << "Segmentation results:";
    os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(2);

    int modifyOld=this->CalibrationPointListNode->StartModify();  
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {
      double r, a, s;
      bool valid;
      robot->GetCalibrationMarker(i, r, a, s, valid);
      if (valid)
      {
        os << std::endl << "  Marker "<<i+1<<": R="<<r<<" A="<<a<<" S="<<s;
      }

      if (this->CalibrationPointListNode->GetNumberOfFiducials()<(int)i+1)
      {    
        this->CalibrationPointListNode->AddFiducialWithLabelXYZSelectedVisibility(MARKER_LABEL_NAMES[i],r,a,s,true,true);
      }
      else
      {
        float* fidXYZ=this->CalibrationPointListNode->GetNthFiducialXYZ(i);
        if (fidXYZ[0]!=r || fidXYZ[1]!=a || fidXYZ[2]!=s)
        {
          this->CalibrationPointListNode->SetNthFiducialXYZ(i, r, a, s);
        }
      }
    }
    //this->CalibrationPointListNode->SetLocked(true);
    this->CalibrationPointListNode->EndModify(modifyOld); 
    // StartModify/EndModify discarded vtkMRMLFiducialListNode::FiducialModifiedEvent-s, so we have to resubmit them now
    this->CalibrationPointListNode->InvokeEvent(vtkMRMLFiducialListNode::FiducialModifiedEvent, NULL);
  }
  else
  {
    os << "Not calibrated.";
    //this->CalibrationPointListNode->SetAllFiducialsVisibility(false);
  }

  os << std::ends;
  this->CalibrationResultsBox->GetWidget()->SetText(os.str());
  os.rdbuf()->freeze();
}
//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::AddGUIObservers()
{
  this->RemoveGUIObservers();
  this->ResetCalibrationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->EditMarkerPositionButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AutomaticCenterpointAdjustmentCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ResegmentButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadCalibrationVolumeButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->VolumeSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->JumpToFiducialButton[i]->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  }
}
//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::RemoveGUIObservers()
{
  this->ResetCalibrationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);  
  this->EditMarkerPositionButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AutomaticCenterpointAdjustmentCheckButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);  
  this->ResegmentButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadCalibrationVolumeButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->VolumeSelectorWidget->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent,  (vtkCommand *)this->GUICallbackCommand );
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->JumpToFiducialButton[i]->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  }
}
//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::Enter()
{  

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::Exit()
{

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::WizardGUICallback( vtkObject *caller,
                         unsigned long eid, void *clientData, void *callData )
{

  vtkProstateNavFiducialCalibrationStep *self = reinterpret_cast<vtkProstateNavFiducialCalibrationStep *>(clientData);
  
  //if (self->GetInGUICallbackFlag())
    {
    // check for infinite recursion here
    }

  vtkDebugWithObjectMacro(self, "In vtkTRProstateCalibrationBiopsyStep GUICallback");
  
  //self->SetInGUICallbackFlag(1);
  self->ProcessGUIEvents(caller, eid, callData);
  //self->SetInGUICallbackFlag(0);
}
//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::UpdateMRML() 
{

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::UpdateGUI() 
{
  PopulateCalibrationResults();
  
  // Update automatic centerpoint adjustment checkbox
  bool enableAutomaticCenterpointAdjustmentOnGui=(this->AutomaticCenterpointAdjustmentCheckButton->GetSelectedState()==1);
  if (enableAutomaticCenterpointAdjustmentOnGui!=this->EnableAutomaticCenterpointAdjustment)
  {
    this->AutomaticCenterpointAdjustmentCheckButton->SetSelectedState(this->EnableAutomaticCenterpointAdjustment?1:0);
  }
} 

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ProcessGUIEvents(vtkObject *caller,
        unsigned long event, void *callData)
{
  //slicerCerr("vtkProstateNavFiducialCalibrationStep::ProcessGUIEvents");

  vtkMRMLProstateNavManagerNode *mrmlNode = this->GetGUI()->GetProstateNavManager();

  if(!mrmlNode)
      return;
  
  // load calib volume dialog button
  if (this->LoadCalibrationVolumeButton && this->LoadCalibrationVolumeButton == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
    {
    this->GetApplication()->Script("::LoadVolume::ShowDialog");
    }

  // Calibrate
  if (this->EditMarkerPositionButton && this->EditMarkerPositionButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    EnableMarkerPositionEdit(this->EditMarkerPositionButton->GetSelectedState() == 1);
    }  

  // Enable/disable automatic marker centerpoint adjustment
  if (this->AutomaticCenterpointAdjustmentCheckButton && this->AutomaticCenterpointAdjustmentCheckButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
      bool newEnable=(this->AutomaticCenterpointAdjustmentCheckButton->GetSelectedState() == 1);
      if (newEnable!=this->EnableAutomaticCenterpointAdjustment)
      {
        this->EnableAutomaticCenterpointAdjustment=newEnable;
        this->Resegment();
      }      
    }

  // reset calib button
  if (this->ResetCalibrationButton && this->ResetCalibrationButton == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
    {
    this->Reset();
    }
  // resegment 
  if (this->ResegmentButton && this->ResegmentButton == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
    {
    this->Resegment();
    }  

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    if (this->JumpToFiducialButton[i] && this->JumpToFiducialButton[i] == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
    {
      this->JumpToFiducial(i);
    }  
  }

  if (this->VolumeSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
  {
    vtkMRMLScalarVolumeNode *volume = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (volume != NULL)
    {
      this->GetGUI()->GetLogic()->SelectVolumeInScene(volume, VOL_CALIBRATION);
      this->EditMarkerPositionButton->SetSelectedState(1);

      // Update the fiducials to use the same transform 
      vtkMRMLScene* scene=NULL;
      if (this->GetLogic()
        && this->GetLogic()->GetApplicationLogic()
        && this->GetLogic()->GetApplicationLogic()->GetMRMLScene())
      {
        scene=this->GetLogic()->GetApplicationLogic()->GetMRMLScene();
      }
      if (scene!=NULL)
      {
        vtkMRMLScalarVolumeNode *calVolNode=vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID(mrmlNode->GetCalibrationVolumeNodeID()));    
        if (calVolNode!=NULL)
        {
          this->CalibrationPointListNode->SetAndObserveTransformNodeID(calVolNode->GetTransformNodeID());
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::Reset()
{
  vtkMRMLTransRectalProstateRobotNode* robot=GetRobot();
  if (robot==NULL)
  {
    return;
  }
  robot->RemoveAllCalibrationMarkers();

  this->CalibrationPointListNode->RemoveAllFiducials();

  const TRProstateBiopsyCalibrationData calibData=robot->GetCalibrationData();
  TRProstateBiopsyCalibrationData calibdataInvalidated=calibData;
  calibdataInvalidated.CalibrationValid=false;
  robot->SetCalibrationData(calibdataInvalidated);

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->SetErrorText("");

  PopulateCalibrationResults();
}
//-----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::Resegment()
{
  vtkMRMLProstateNavManagerNode *mrmlNode = this->GetGUI()->GetProstateNavManager();

  if(!mrmlNode)
  {
    vtkErrorMacro("Resegment: node is invalid");
    return;
  }

  vtkMRMLTransRectalProstateRobotNode* robot=GetRobot();
  if (robot==NULL)
  {    
    vtkErrorMacro("Resegment: robot is invalid");
    return;
  }

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  std::ostrstream os;

  if (this->CalibrationPointListNode->GetNumberOfFiducials()<(int)CALIB_MARKER_COUNT)
  {
    os << "Please define all calibration markers." << std::ends;
    wizard_widget->SetErrorText(os.str());
    os.rdbuf()->freeze();
    wizard_widget->Update();
    return;
  }

  os << "Starting segmentation..." << std::ends;
  wizard_widget->SetErrorText(os.str());
  os.rdbuf()->freeze();
  wizard_widget->Update();

  // gather information about thresholds
  double thresh[CALIB_MARKER_COUNT];
  for (unsigned int i=0 ; i<CALIB_MARKER_COUNT; i++)
  {
    thresh[i] = this->FiducialThresholdScale[i]->GetValue();
  }
  // gather info about the fiducial dimenstions
  double fidDims[3];
  fidDims[0] = this->FiducialWidthSpinBox->GetWidget()->GetValue();
  fidDims[1] = this->FiducialHeightSpinBox->GetWidget()->GetValue();
  fidDims[2] = this->FiducialDepthSpinBox->GetWidget()->GetValue();

  // gather info about initial radius, and initial angle
  double radius = 0;
  bool bUseRadius = false;
  double initialAngle = 0;

  radius = this->RadiusSpinBox->GetWidget()->GetValue();
  bUseRadius = this->RadiusCheckButton->GetSelectedState();
  initialAngle = this->InitialAngleSpinBox->GetWidget()->GetValue();

  // start the segmentation/registration
  //bool found[CALIB_MARKER_COUNT]={false, false, false, false};
  std::string calibResultDetails;

  const char* calVolNodeID = mrmlNode->GetCalibrationVolumeNodeID();
  vtkMRMLScalarVolumeNode *calVolNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(calVolNodeID));

  bool calibResult=robot->SegmentRegisterMarkers(calVolNode, thresh, fidDims, radius, bUseRadius, initialAngle, calibResultDetails, this->EnableAutomaticCenterpointAdjustment);
  
  wizard_widget->SetErrorText(calibResultDetails.c_str());
  wizard_widget->Update();
 
  ShowMarkerVolumesIn3DView(true);

  if (calibResult==true)
  {
    // calibration successful  
    this->ShowAxesIn3DView(true);
  }
  else
  {
    // calibration failed
    this->ShowAxesIn3DView(false);
  }

  GetMainViewerWidget()->RequestRender();

  //this->CalibrationPointListNode->SetAllFiducialsVisibility(false);

  UpdateGUI(); // TODO: this should be called automatically, as the robot node is changed
}

//--------------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ShowAxesIn3DView(bool show)
{
  vtkMRMLTransRectalProstateRobotNode* robot = GetRobot();    
  if (robot)
  {
    robot->SetModelAxesVisible(show);
  }
}

void vtkProstateNavFiducialCalibrationStep::ShowMarkerVolumesIn3DView(bool /*show*/)
{
  // :TODO: implement if needed (currently they are always shown)
}

vtkMRMLTransRectalProstateRobotNode* vtkProstateNavFiducialCalibrationStep::GetRobot()
{
  if (this->GetGUI()==NULL)
  {
    return NULL;
  }
  if (this->GetGUI()->GetProstateNavManager()==NULL)
  {
    return NULL;
  }
  return vtkMRMLTransRectalProstateRobotNode::SafeDownCast(this->GetGUI()->GetProstateNavManager()->GetRobotNode());
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::ProcessMRMLEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{
  if (caller == (vtkObject*)this->CalibrationPointListNode)
  {
    switch (event)
    {
    case vtkCommand::ModifiedEvent: // Modified Event
    case vtkMRMLScene::NodeAddedEvent: // when a fiducial is added to the list
    case vtkMRMLFiducialListNode::FiducialModifiedEvent:
    case vtkMRMLFiducialListNode::DisplayModifiedEvent:
      UpdateCalibration();
      break;
    }
  }

  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && event == vtkMRMLScene::NodeAddedEvent )
    {
    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast((vtkMRMLNode*)(callData));
    if (volumeNode!=NULL && this->VolumeSelectorWidget!=NULL && volumeNode!=this->VolumeSelectorWidget->GetSelected() )
      {
      // a new volume is loaded, set as the current calibration volume
      this->VolumeSelectorWidget->SetSelected(volumeNode);
      }
    }

  vtkMRMLTransRectalProstateRobotNode *robot = vtkMRMLTransRectalProstateRobotNode::SafeDownCast(caller);
  if (robot!=NULL && robot == GetRobot() && event == vtkCommand::ModifiedEvent)
    {
    UpdateGUI();
    }

}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::AddMRMLObservers()
{
  RemoveMRMLObservers(); // remove old observers first to make sure that there are no duplicate observers
  this->CalibrationPointListNode->AddObserver(vtkCommand::ModifiedEvent,this->MRMLCallbackCommand);
  this->CalibrationPointListNode->AddObserver(vtkMRMLScene::NodeAddedEvent,this->MRMLCallbackCommand);
  this->CalibrationPointListNode->AddObserver(vtkMRMLFiducialListNode::DisplayModifiedEvent,this->MRMLCallbackCommand);
  this->CalibrationPointListNode->AddObserver(vtkMRMLFiducialListNode::FiducialModifiedEvent,this->MRMLCallbackCommand);

  if (this->GetProstateNavManager())
  {
    vtkMRMLTransRectalProstateRobotNode* robot = GetRobot();    
    if (robot)
    {
      this->ObservedRobot = robot;
      this->ObservedRobot->Register(this); // prevent deletion of the object while we are still holding a reference to it
      this->ObservedRobot->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
    }
    else
    {
      this->ObservedRobot = NULL;
    }    
  }

  if (this->MRMLScene!=NULL)
  {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
  }
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::RemoveMRMLObservers()
{
  this->CalibrationPointListNode->RemoveObserver(this->MRMLCallbackCommand);
  if (this->ObservedRobot!=NULL)
  {       
    this->ObservedRobot->RemoveObserver(this->MRMLCallbackCommand);
    this->ObservedRobot->UnRegister(this);
    this->ObservedRobot = NULL;    
  }
  if (this->MRMLScene!=NULL)
  {
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
  }
}

//----------------------------------------------------------------------------
void vtkProstateNavFiducialCalibrationStep::EnableMarkerPositionEdit(bool enable)
{  
  // Activate target fiducials in the Fiducial GUI
  if (this->GetLogic()==NULL)
  {
    vtkErrorMacro("Logic is invalid");
    return;
  }    

  if (enable)
  {
    // Set fiducial placement mode
    GetLogic()->SetCurrentFiducialList(this->CalibrationPointListNode);
    if (this->CalibrationPointListNode->GetNumberOfFiducials()<(int)CALIB_MARKER_COUNT)
    {
      GetLogic()->SetMouseInteractionMode(vtkMRMLInteractionNode::Place); 
    }
    else
    {
      GetLogic()->SetMouseInteractionMode(vtkMRMLInteractionNode::ViewTransform); 
    }
    this->CalibrationPointListNode->SetLocked(false);
  }
  else
  {
    GetLogic()->SetMouseInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    this->CalibrationPointListNode->SetLocked(true);
  }

}


void vtkProstateNavFiducialCalibrationStep::UpdateCalibration()
{  

  vtkMRMLTransRectalProstateRobotNode* robot= GetRobot();
  if(robot==NULL)
  {
    vtkErrorMacro("UpdateCalibration: node is invalid");
    return;
  }

  // remove extra markers
  for (int i=CALIB_MARKER_COUNT; i<this->CalibrationPointListNode->GetNumberOfFiducials(); i++)
  {
    // this is an extra marker => delete it
    this->CalibrationPointListNode->RemoveFiducial(i);
  }

  // make sure that markers have proper names and their values are set in the robot node
  int fidCount=this->CalibrationPointListNode->GetNumberOfFiducials();
  for (int i=0; i<fidCount; i++)
  {
    float* rasPoint=this->CalibrationPointListNode->GetNthFiducialXYZ(i);
    double rasPointDouble[3]={rasPoint[0], rasPoint[1], rasPoint[2]};
    robot->SetCalibrationMarker(i, rasPointDouble);
    if (strcmp(this->CalibrationPointListNode->GetNthFiducialLabelText(i),MARKER_LABEL_NAMES[i])!=0)
    {
      this->CalibrationPointListNode->SetNthFiducialLabelText(i,MARKER_LABEL_NAMES[i]);      
    }
  }

  // Update marker positioning mode (as new points are added the mode should be changed from "place new points" to "edit points"
  EnableMarkerPositionEdit(this->EditMarkerPositionButton->GetSelectedState() == 1);

  if (this->CalibrationPointListNode->GetNumberOfFiducials()<(int)CALIB_MARKER_COUNT)
  {
    // not enough fiducials to compute result
    return;
  }

  this->Resegment();
}

void vtkProstateNavFiducialCalibrationStep::JumpToFiducial(unsigned int fid1Index)
{
  if (fid1Index>=CALIB_MARKER_COUNT)
  {
    vtkErrorMacro("Invalid fiducial id "<<fid1Index);
    return;
  }

  if ((int)fid1Index>=this->CalibrationPointListNode->GetNumberOfFiducials())
  {
    vtkErrorMacro("Fiducial is not defined yet "<<fid1Index);
    return;
  }

  vtkMRMLTransRectalProstateRobotNode* robot=GetRobot();
  if(robot==NULL)
  {
    vtkErrorMacro("UpdateCalibration: node is invalid");
    return;
  }

  // get the index of the fiducial pair (on the same line)
  int fid2Index=-1;
  if (fid1Index<2)
  {
    // probe fiducial
    if (fid1Index==0)
    {
      fid2Index=1;
    }
    else
    {
      fid2Index=0;
    }
  }
  else
  {
    // needle fiducial
    if (fid1Index==2)
    {
      fid2Index=3;
    }
    else
    {
      fid2Index=2;
    }
  }

  float* fid1PosFloat=this->CalibrationPointListNode->GetNthFiducialXYZ(fid1Index);
  
  double fid1Pos[4]={fid1PosFloat[0], fid1PosFloat[1], fid1PosFloat[2]};

  if (fid2Index<this->CalibrationPointListNode->GetNumberOfFiducials())
  {
    double n[4]={1,0,0,0}; // plane normal
    double t[4]={0,1,0,0}; // a transverse plane

    float* fid2PosFloat=this->CalibrationPointListNode->GetNthFiducialXYZ(fid2Index);
    double fid2Pos[3]={fid2PosFloat[0], fid2PosFloat[1], fid2PosFloat[2]};
    n[0]=fid2Pos[0]-fid1Pos[0];
    n[1]=fid2Pos[1]-fid1Pos[1];
    n[2]=fid2Pos[2]-fid1Pos[2];

    // transverse plane will be an arbitrary plane that is perpendicular to n
    double point1[4]={1,0,0,1}; // Any vector
    if (fabs(n[0])>fabs(n[1]))
    {
      // to make sure that it is not parallel to the planeNormal_RAS vector
      point1[0]=0;
      point1[1]=1;
      point1[2]=0;
    }
    // planeX_RAS is perpendicular to planeNormal_RAS
    vtkMath::Cross(n, point1, t);
    vtkMath::Normalize(t);

    this->GetGUI()->BringMarkerToViewIn2DViews(fid1Pos,n,t);
  }
  else
  {
    this->GetGUI()->BringMarkerToViewIn2DViews(fid1Pos);
  }
 
}

vtkSlicerViewerWidget* vtkProstateNavFiducialCalibrationStep::GetMainViewerWidget()
{
  if (this->GetGUI()==NULL)
  {
    vtkWarningMacro("GUI is null");
    return NULL;
  }
  if (this->GetGUI()->GetApplicationGUI()==NULL)
  {
    vtkWarningMacro("Application GUI is null");
    return NULL;
  }

  // return GetMainViewerWidget(); // for earlier Slicer versions (without multiple camera support)

  if (this->GetGUI()->GetApplicationGUI()->GetNumberOfViewerWidgets()<1)
  {
    vtkWarningMacro("There is no viewer widget");
    return NULL;
  }
  return this->GetGUI()->GetApplicationGUI()->GetNthViewerWidget(0); // main viewer is the first viewer
}
