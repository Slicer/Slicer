#include "vtkEMSegmentIntensityNormalizationStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWScaleWithEntry.h"

#include "vtkKWHistogram.h"
#include "vtkKWPiecewiseFunctionEditor.h"
//#include "vtkKWPiecewiseFunction.h"
#include "vtkPointData.h"
#include "vtkImageMeanIntensityPreNormalization.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityNormalizationStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityNormalizationStep, "$Revision: 1.1 $");
//----------------------------------------------------------------------------
vtkEMSegmentIntensityNormalizationStep::vtkEMSegmentIntensityNormalizationStep()
{
  this->SetName("5/9. Intensity Normalization");
  this->SetDescription("Apply intensity normalization to target images.");

  this->NormalizationTargetVolumeMenuButton  = NULL;
  this->NormalizationParametersFrame         = NULL;
  this->NormalizationEnableCheckButton       = NULL;
  this->NormalizationDefaultsMenuButton      = NULL;
  this->NormalizationPrintCheckButton        = NULL;
  this->NormalizationNormValueEntry          = NULL;
  this->NormalizationSmoothingWidthEntry     = NULL;
  this->NormalizationMaxSmoothingWidthEntry  = NULL;
  this->NormalizationRelativeMaxVoxelScale   = NULL;
  
  this->NormalizationHistogramFrame          = NULL;
  this->NormalizationHistogram               = NULL;
  this->NormalizationHistogramMenuButton     = NULL;
  this->VisuHisto                            = NULL;
  this->NormalizationValueRecommandedEntry   = NULL;
  
  this->RecommandationFrame          = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityNormalizationStep::~vtkEMSegmentIntensityNormalizationStep()
{
  if(this->NormalizationTargetVolumeMenuButton)
    {
    this->NormalizationTargetVolumeMenuButton->Delete();
    this->NormalizationTargetVolumeMenuButton = NULL;
    }
  if(this->NormalizationEnableCheckButton)
    {
    this->NormalizationEnableCheckButton->Delete();
    this->NormalizationEnableCheckButton = NULL;
    }
  if(this->NormalizationDefaultsMenuButton)
    {
    this->NormalizationDefaultsMenuButton->Delete();
    this->NormalizationDefaultsMenuButton = NULL;
    }
  if(this->NormalizationPrintCheckButton)
    {
    this->NormalizationPrintCheckButton->Delete();
    this->NormalizationPrintCheckButton = NULL;
    }
  if(this->NormalizationNormValueEntry)
    {
    this->NormalizationNormValueEntry->Delete();
    this->NormalizationNormValueEntry = NULL;
    }
  if(this->NormalizationSmoothingWidthEntry)
    {
    this->NormalizationSmoothingWidthEntry->Delete();
    this->NormalizationSmoothingWidthEntry = NULL;
    }
  if(this->NormalizationMaxSmoothingWidthEntry)
    {
    this->NormalizationMaxSmoothingWidthEntry->Delete();
    this->NormalizationMaxSmoothingWidthEntry = NULL;
    }
  if(this->NormalizationRelativeMaxVoxelScale)
    {
    this->NormalizationRelativeMaxVoxelScale->Delete();
    this->NormalizationRelativeMaxVoxelScale = NULL;
    }
  if(this->NormalizationParametersFrame)
    {
    this->NormalizationParametersFrame->Delete();
    this->NormalizationParametersFrame = NULL;
    }
    
    
    
   if(this->NormalizationHistogram)
    {
    this->NormalizationHistogram->Delete();
    this->NormalizationHistogram = NULL;
    }
   if(this->NormalizationHistogramFrame)
    {
    this->NormalizationHistogramFrame->Delete();
    this->NormalizationHistogramFrame = NULL;
    }
   if(this->NormalizationHistogramMenuButton)
    {
    this->NormalizationHistogramMenuButton->Delete();
    this->NormalizationHistogramMenuButton = NULL;
    }
   if(this->VisuHisto)
    {
    this->VisuHisto->Delete();
    this->VisuHisto = NULL;
    }
   if(this->NormalizationValueRecommandedEntry)
    {
    this->NormalizationValueRecommandedEntry->Delete();
    this->NormalizationValueRecommandedEntry = NULL;
    }
    
   if(this->RecommandationFrame)
    {
    this->RecommandationFrame->Delete();
    this->RecommandationFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Create the target images volume selector

  if (!this->NormalizationTargetVolumeMenuButton)
    {
    this->NormalizationTargetVolumeMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->NormalizationTargetVolumeMenuButton->IsCreated())
    {
    this->NormalizationTargetVolumeMenuButton->SetParent(parent);
    this->NormalizationTargetVolumeMenuButton->Create();
    this->NormalizationTargetVolumeMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->NormalizationTargetVolumeMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->NormalizationTargetVolumeMenuButton->
      SetLabelText("Target Image:");
    this->NormalizationTargetVolumeMenuButton->
      SetBalloonHelpString("Select a target image to set intensity normalization parameters.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 5", 
    this->NormalizationTargetVolumeMenuButton->GetWidgetName());
  
  this->PopulateNormalizationTargetVolumeSelector();
  
  // Create the normalization parameters frame

  if (!this->NormalizationParametersFrame)
    {
    this->NormalizationParametersFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NormalizationParametersFrame->IsCreated())
    {
    this->NormalizationParametersFrame->SetParent(parent);
    this->NormalizationParametersFrame->Create();
    this->NormalizationParametersFrame->SetLabelText(
      "Normalization Parameters");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->NormalizationParametersFrame->GetWidgetName());

  vtkKWFrame* parentFrame = this->NormalizationParametersFrame->GetFrame();

  // Create the enable-normalization check button

  if (!this->NormalizationEnableCheckButton)
    {
    this->NormalizationEnableCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NormalizationEnableCheckButton->IsCreated())
    {
    this->NormalizationEnableCheckButton->SetParent(parentFrame);
    this->NormalizationEnableCheckButton->Create();
    this->NormalizationEnableCheckButton->SetLabelText(
      "Enable Normalization:");
    this->NormalizationEnableCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 8);
    }
  
  this->NormalizationEnableCheckButton->GetWidget()->SetCommand(NULL, NULL);
  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2", 
               this->NormalizationEnableCheckButton->GetWidgetName());
  
  vtkKWMenu *menu = this->NormalizationTargetVolumeMenuButton->
      GetWidget()->GetMenu();

  // Create the normalization actions menu button

  if (!this->NormalizationDefaultsMenuButton)
    {
    this->NormalizationDefaultsMenuButton =
      vtkKWMenuButton::New();
    }
  if (!this->NormalizationDefaultsMenuButton->IsCreated())
    {
    this->NormalizationDefaultsMenuButton->SetParent(parentFrame);
    this->NormalizationDefaultsMenuButton->Create();
    this->NormalizationDefaultsMenuButton->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH-3);
    }
  this->NormalizationDefaultsMenuButton->SetConfigurationOption(
    "-text", "Reset Defaults");
  this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2", 
    this->NormalizationDefaultsMenuButton->GetWidgetName());

  // Create the print-info check button

  if (!this->NormalizationPrintCheckButton)
    {
    this->NormalizationPrintCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NormalizationPrintCheckButton->IsCreated())
    {
    this->NormalizationPrintCheckButton->SetParent(parentFrame);
    this->NormalizationPrintCheckButton->Create();
    this->NormalizationPrintCheckButton->SetLabelText(
      "Print Info:");
    this->NormalizationPrintCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 8);
    }
  
  this->NormalizationPrintCheckButton->GetWidget()->SetCommand(NULL, NULL);
  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2", 
               this->NormalizationPrintCheckButton->GetWidgetName());

  // Create the Norm value entry

  if (!this->NormalizationNormValueEntry)
    {
    this->NormalizationNormValueEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->NormalizationNormValueEntry->IsCreated())
    {
    this->NormalizationNormValueEntry->SetParent(parentFrame);
    this->NormalizationNormValueEntry->Create();
    this->NormalizationNormValueEntry->SetLabelText("Norm Value:");
    this->NormalizationNormValueEntry->SetLabelWidth(EMSEG_WIDGETS_LABEL_WIDTH-4);
    vtkKWEntry *entry = this->NormalizationNormValueEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToDouble();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 2 -sticky nw -padx 2 -pady 2",
               this->NormalizationNormValueEntry->GetWidgetName());

  // Create the Histogram smoothing width entry

  if (!this->NormalizationSmoothingWidthEntry)
    {
    this->NormalizationSmoothingWidthEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->NormalizationSmoothingWidthEntry->IsCreated())
    {
    this->NormalizationSmoothingWidthEntry->SetParent(parentFrame);
    this->NormalizationSmoothingWidthEntry->Create();
    this->NormalizationSmoothingWidthEntry->SetLabelText("Histogram Smoothing Width:");
    this->NormalizationSmoothingWidthEntry->SetLabelWidth(EMSEG_WIDGETS_LABEL_WIDTH -4);
    vtkKWEntry *entry = this->NormalizationSmoothingWidthEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 3 -sticky nw -padx 2 -pady 2",
               this->NormalizationSmoothingWidthEntry->GetWidgetName());

  // Create the Histogram smoothing max entry

  if (!this->NormalizationMaxSmoothingWidthEntry)
    {
    this->NormalizationMaxSmoothingWidthEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->NormalizationMaxSmoothingWidthEntry->IsCreated())
    {
    this->NormalizationMaxSmoothingWidthEntry->SetParent(parentFrame);
    this->NormalizationMaxSmoothingWidthEntry->Create();
    this->NormalizationMaxSmoothingWidthEntry->SetLabelText("Max:");
    this->NormalizationMaxSmoothingWidthEntry->SetLabelWidth(5);
    vtkKWEntry *entry = this->NormalizationMaxSmoothingWidthEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 3 -sticky se -padx 5 -pady 2",
               this->NormalizationMaxSmoothingWidthEntry->GetWidgetName());

  // Create the Relative max voxel number scale

  if (!this->NormalizationRelativeMaxVoxelScale)
    {
    this->NormalizationRelativeMaxVoxelScale = vtkKWScaleWithEntry::New();
    }
  if (!this->NormalizationRelativeMaxVoxelScale->IsCreated())
    {
    this->NormalizationRelativeMaxVoxelScale->SetParent(parentFrame);
    this->NormalizationRelativeMaxVoxelScale->PopupModeOn();
    this->NormalizationRelativeMaxVoxelScale->Create();
    this->NormalizationRelativeMaxVoxelScale->SetEntryWidth(4);
    this->NormalizationRelativeMaxVoxelScale->SetLabelText("Relative Max Voxel Number:");
    this->NormalizationRelativeMaxVoxelScale->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-4);
    this->NormalizationRelativeMaxVoxelScale->SetRange(0.0, 1.0);
    this->NormalizationRelativeMaxVoxelScale->SetResolution(0.01);
    this->NormalizationRelativeMaxVoxelScale->GetEntry()->
      SetRestrictValueToDouble();
    this->NormalizationRelativeMaxVoxelScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 4 -sticky nw -padx 2 -pady 2", 
               this->NormalizationRelativeMaxVoxelScale->GetWidgetName());

  this->NormalizationEnableCheckButton->SetEnabled(0);
  this->NormalizationDefaultsMenuButton->SetEnabled(0);
  this->NormalizationPrintCheckButton->SetEnabled(0);
  this->NormalizationNormValueEntry->SetEnabled(0);
  this->NormalizationSmoothingWidthEntry->SetEnabled(0);
  this->NormalizationMaxSmoothingWidthEntry->SetEnabled(0);
  this->NormalizationRelativeMaxVoxelScale->SetEnabled(0);

  vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager0)
    {
    return;
    }
  this->NormalizationTargetVolumeMenuButton->SetEnabled(
    mrmlManager0->GetTargetNumberOfSelectedVolumes() ? parent->GetEnabled() : 0);

  if(this->NormalizationTargetVolumeMenuButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    
    if(!mrmlManager->GetTargetNumberOfSelectedVolumes() ||
      menu->GetNumberOfItems()<=0)
      {
      this->NormalizationTargetVolumeMenuButton->
        GetWidget()->SetValue("");
      }
    else
      {
      menu->SelectItem(0);
      this->NormalizationTargetSelectionChangedCallback(
        mrmlManager->GetTargetSelectedVolumeNthID(0));
      }
    }
        
      // Create the histogram frame

  if (!this->NormalizationHistogramFrame)
    {
    this->NormalizationHistogramFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NormalizationHistogramFrame->IsCreated())
    {
    this->NormalizationHistogramFrame->SetParent(parent);
    this->NormalizationHistogramFrame->Create();
    this->NormalizationHistogramFrame->SetLabelText(
      "Histogram");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->NormalizationHistogramFrame->GetWidgetName());

  vtkKWFrame* userFrame = this->NormalizationHistogramFrame->GetFrame();
  

    // Create the histogram volume selector

  if (!this->NormalizationHistogramMenuButton)
    {
    this->NormalizationHistogramMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->NormalizationHistogramMenuButton->IsCreated())
    {
    this->NormalizationHistogramMenuButton->SetParent(userFrame);//parent
    this->NormalizationHistogramMenuButton->Create();
    this->NormalizationHistogramMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->NormalizationHistogramMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->NormalizationHistogramMenuButton->
      SetLabelText("Atlas Image:");
    this->NormalizationHistogramMenuButton->
      SetBalloonHelpString("Select a target image find intensity normalization parameters.");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    this->NormalizationHistogramMenuButton->GetWidgetName());
  
  this->PopulateNormalizationHistogramSelector();
    
  //update everything
    
  // Create the histogram
     
     
   if (!this->NormalizationHistogram)
    {
    this->NormalizationHistogram = vtkKWHistogram::New();
    this->VisuHisto = vtkKWPiecewiseFunctionEditor::New();
    }
     
   if (!this->VisuHisto->IsCreated())
    {
    this->VisuHisto->SetParent(userFrame);//parent
    this->VisuHisto->Create();
    this->VisuHisto->GetTclName();
    this->VisuHisto->SetBorderWidth(2);
    //this->VisuHisto->SetReliefToGroove();
    this->VisuHisto->SetPadX(2);
    this->VisuHisto->SetPadY(2);
    
    this->VisuHisto->ParameterTicksVisibilityOn();
    this->VisuHisto->ValueTicksVisibilityOn();
    this->VisuHisto->ComputeValueTicksFromHistogramOn();
    this->VisuHisto->SetParameterTicksFormat("%-#6.0f");
    this->VisuHisto->SetValueTicksFormat(
    this->VisuHisto->GetParameterTicksFormat());
    /*
    
    this->VisuHisto->ParameterRangeVisibilityOn();
    this->VisuHisto->ExpandCanvasWidthOn();
    this->VisuHisto->SetCanvasWidth(300);
    this->VisuHisto->SetCanvasHeight(150);
    this->VisuHisto->SetLabelText("Histogram:");
    this->VisuHisto->LabelVisibilityOn();*/
    this->VisuHisto->SetFrameBackgroundColor(0.92, 1.0, 0.92);
    /*
    //this->VisuHisto->SetLabelText("Histogram");
    //this->VisuHisto->SetBalloonHelpString("Valeur du pixel");
    //this->VisuHisto->ValueRangeVisibilityOn();
    //this->VisuHisto->ParameterRangeVisibilityOn();
    this->VisuHisto->SetLabelPositionToTop();
    //this->VisuHisto->LockEndPointsParameterOff();
    */
    this->VisuHisto->SetParameterCursorVisibility(50);
    this->VisuHisto->SetParameterCursorPosition(50);
    this->VisuHisto->SetParameterCursorInteractionStyle(7);
    //this->VisuHisto->SetParameterCursorMovingCommand(this,"ParameterCursorMovingCommand");      
    }
     
     
     
     
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2", 
    this->VisuHisto->GetWidgetName());
    
   
   
 
     
    this->NormalizationHistogramMenuButton->SetEnabled(
    mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->NormalizationHistogramMenuButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int vol_id = mrmlManager->GetVolumeNthID(0);
    this->NormalizationHistogramChangedCallback(vol_id);
    }
    
    this->AddCursorMovingGUIEvents(); 
    
    
    
    
    
    
          // Create the recommandation frame

  if (!this->RecommandationFrame)
    {
    this->RecommandationFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RecommandationFrame->IsCreated())
    {
    this->RecommandationFrame->SetParent(parent);
    this->RecommandationFrame->Create();
    this->RecommandationFrame->SetBackgroundColor(1,0.56,0);
    this->RecommandationFrame->SetLabelText(
      "Recommandations");

      
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->RecommandationFrame->GetWidgetName());
    
    

  vtkKWFrame* recomFrame = this->RecommandationFrame->GetFrame();
    
 
       // Create the recommanded value

  if (!this->NormalizationValueRecommandedEntry)
    {
    this->NormalizationValueRecommandedEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->NormalizationValueRecommandedEntry->IsCreated())
    {
    this->NormalizationValueRecommandedEntry->SetParent(recomFrame);
    this->NormalizationValueRecommandedEntry->Create();
    this->NormalizationValueRecommandedEntry->SetLabelText("Recom. Norm Value:");
    this->NormalizationValueRecommandedEntry->SetLabelWidth(EMSEG_WIDGETS_LABEL_WIDTH -8);
    vtkKWEntry *entry = this->NormalizationValueRecommandedEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2",
               this->NormalizationValueRecommandedEntry->GetWidgetName());
  
    
     
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  PopulateNormalizationTargetVolumeSelector()
{  
  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  vtkKWMenu* menu = this->NormalizationTargetVolumeMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d", 
            "NormalizationTargetSelectionChangedCallback", 
            static_cast<int>(target_vol_id));
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  PopulateNormalizationHistogramSelector()
{  


  vtkIdType vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager(); 
   
 if (!mrmlManager)
    {
    return;
    }
  int nb_of_volumes = mrmlManager->GetVolumeNumberOfChoices();
  
  vtkKWMenu* menu = this->NormalizationHistogramMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_volumes; i++)
    {
    vol_id = mrmlManager->GetVolumeNthID(i);
    sprintf(buffer, "%s %d", 
            "NormalizationHistogramChangedCallback", 
            static_cast<int>(vol_id));
    const char *name = mrmlManager->GetVolumeName(vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationTargetSelectionChangedCallback(vtkIdType target_vol_id)
{
  // The target volumes have changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  
  char buffer[256];
  int parentEnabled = 
    this->NormalizationParametersFrame->GetEnabled();

  // Update Normalization-Enable check button

  this->NormalizationEnableCheckButton->SetEnabled(parentEnabled);  
  vtkKWCheckButton *cbEnable = 
    this->NormalizationEnableCheckButton->GetWidget();
  sprintf(buffer, "NormalizationEnableCallback %d", 
          static_cast<int>(target_vol_id));
  cbEnable->SetCommand(this, buffer);
  cbEnable->SetSelectedState(mrmlManager->
    GetTargetVolumeIntensityNormalizationEnabled(target_vol_id));
  this->NormalizationEnableCallback(
    target_vol_id, cbEnable->GetSelectedState());
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationHistogramChangedCallback(vtkIdType target_vol_id)
{
  // The target volumes have changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(target_vol_id);
  
  vtkImageData* inputImage = volumeNode->GetImageData();
  
  vtkDataArray* array = inputImage->GetPointData()->GetScalars();
  
  this->NormalizationHistogram->BuildHistogram(array,0);
  this->VisuHisto->SetHistogram(NormalizationHistogram);
  this->VisuHisto->DisplayHistogramOnly();
  
  
  this->VisuHisto->SetWholeParameterRangeToFunctionRange();
  this->VisuHisto->SetVisibleParameterRangeToWholeParameterRange();
  this->VisuHisto->ParameterRangeVisibilityOn();
  this->VisuHisto->ExpandCanvasWidthOn();
  //this->VisuHisto->SetCanvasWidth(200);
  this->VisuHisto->SetCanvasHeight(180);
  
  id_event = target_vol_id;
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationEnableCallback(vtkIdType target_vol_id, int checked)
{
  // The target volume enabled checkbutton has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTargetVolumeIntensityNormalizationEnabled(target_vol_id, 
    checked);

  int iEnabled = 
    this->NormalizationParametersFrame->GetEnabled();
  if(!checked)
    {
    iEnabled = 0;
    }
  this->NormalizationDefaultsMenuButton->SetEnabled(iEnabled);
  this->NormalizationPrintCheckButton->SetEnabled(iEnabled);
  this->NormalizationNormValueEntry->SetEnabled(iEnabled);
  this->NormalizationSmoothingWidthEntry->SetEnabled(iEnabled);
  this->NormalizationMaxSmoothingWidthEntry->SetEnabled(iEnabled);
  this->NormalizationRelativeMaxVoxelScale->SetEnabled(iEnabled);

  if(!iEnabled)
    {
    return;
    }

  // Update Normalization default settings drop down
  char buffer[256];

  this->NormalizationDefaultsMenuButton->GetMenu()->DeleteAllItems();
  sprintf(buffer, "NormalizationNormTypeCallback %d %d", 
          static_cast<int>(target_vol_id), NormalizationDefaultT1SPGR);
  this->NormalizationDefaultsMenuButton->
    GetMenu()->AddRadioButton("MR T1 SPGR", this, buffer);
  sprintf(buffer, "NormalizationNormTypeCallback %d %d", 
          static_cast<int>(target_vol_id), NormalizationDefaultT2);
  this->NormalizationDefaultsMenuButton->
    GetMenu()->AddRadioButton("MR T2", this, buffer);

  // Update Print-Info check button

  sprintf(buffer, "NormalizationPrintInfoCallback %d", 
          static_cast<int>(target_vol_id));
  this->NormalizationPrintCheckButton->GetWidget()->SetCommand(
    this, buffer);

  // Update Norm-value entry

  vtkKWEntry *entry = this->NormalizationNormValueEntry->GetWidget();
  sprintf(buffer, "NormalizationNormValueCallback %d", 
          static_cast<int>(target_vol_id));
  entry->SetCommand(this, buffer);

  // Update Histogram-smoothing-width

  entry = this->NormalizationSmoothingWidthEntry->GetWidget();
  sprintf(buffer, "NormalizationSmoothingWidthCallback %d", 
          static_cast<int>(target_vol_id));
  entry->SetCommand(this, buffer);

  // Update Max Histogram-smoothing-width

  entry = this->NormalizationMaxSmoothingWidthEntry->GetWidget();
  sprintf(buffer, "NormalizationMaxSmoothingWidthCallback %d", 
          static_cast<int>(target_vol_id));
  entry->SetCommand(this, buffer);

  // Update the Relative-Max-Voxel Num scale entry.

  sprintf(
    buffer, "NormalizationRelativeMaxVoxelNumCallback %d", 
    static_cast<int>(target_vol_id));
  this->NormalizationRelativeMaxVoxelScale->SetEndCommand(this, buffer);
  this->NormalizationRelativeMaxVoxelScale->SetEntryCommand(this, buffer);

  this->ResetDefaultParameters(target_vol_id);

  int parentEnabled = 
    this->NormalizationParametersFrame->GetEnabled();
  vtkKWCheckButton *cbEnable = 
    this->NormalizationEnableCheckButton->GetWidget();

  this->NormalizationDefaultsMenuButton->SetEnabled(
    cbEnable->GetSelectedState() ? parentEnabled : 0);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationNormTypeCallback(vtkIdType target_vol_id,
  int enumDefaultsetting)
{
  // The target volume default settings dropdown has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  
  if (enumDefaultsetting == NormalizationDefaultT1SPGR)
    {
    mrmlManager->SetTargetVolumeIntensityNormalizationToDefaultT1SPGR(
      target_vol_id);
    }
  else if (enumDefaultsetting == NormalizationDefaultT2)
    {
    mrmlManager->SetTargetVolumeIntensityNormalizationToDefaultT2(
      target_vol_id);
    }
  this->ResetDefaultParameters(target_vol_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationPrintInfoCallback(
  vtkIdType target_vol_id, int checked)
{
  // The print-info checkbutton has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
 
  mrmlManager->SetTargetVolumeIntensityNormalizationPrintInfo(
    target_vol_id, checked);

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationNormValueCallback(vtkIdType target_vol_id, double dValue)
{
  // The Norm-value entry has changed 
  // because of user interaction
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
 
  mrmlManager->
    SetTargetVolumeIntensityNormalizationNormValue(target_vol_id, dValue);

}


//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationSmoothingWidthCallback(
  vtkIdType target_vol_id, int iValue)
{
  // The Norm-value entry has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
 
  mrmlManager->
    SetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(
    target_vol_id, iValue);

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationMaxSmoothingWidthCallback(
  vtkIdType target_vol_id, int iValue)
{
  // The Norm-value entry has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
 
  mrmlManager->SetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(
    target_vol_id, iValue);

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationRelativeMaxVoxelNumCallback(
  vtkIdType target_vol_id, double dValue)
{
  // The Norm-value entry has changed 
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
 
  mrmlManager->SetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(
    target_vol_id, dValue);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ResetDefaultParameters(
  vtkIdType target_vol_id)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  this->NormalizationDefaultsMenuButton->SetConfigurationOption(
    "-text", "Reset Defaults");

  this->NormalizationPrintCheckButton->GetWidget()->SetSelectedState(
    mrmlManager->GetTargetVolumeIntensityNormalizationPrintInfo(
    target_vol_id));
  this->NormalizationNormValueEntry->GetWidget()->SetValueAsDouble(
    mrmlManager->GetTargetVolumeIntensityNormalizationNormValue(
    target_vol_id));
  this->NormalizationSmoothingWidthEntry->GetWidget()->SetValueAsInt(
    mrmlManager->
    GetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(
    target_vol_id));
  this->NormalizationMaxSmoothingWidthEntry->GetWidget()->SetValueAsInt(
    mrmlManager->
    GetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(
    target_vol_id));
  this->NormalizationRelativeMaxVoxelScale->
    SetValue(mrmlManager->
    GetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(
    target_vol_id));
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//---------------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::GetValueeee()
{
  std::cout<<"Position1: "<<this->VisuHisto->GetParameterCursorPosition()<<std::endl;
}

//------------------------------------------------------------------------------------------
/*
void vtkEMSegmentIntensityNormalizationStep::InvokeParameterCursorMovingCommand(){

std::cout<<"Position: "<<this->VisuHisto->GetParameterCursorPosition()<<std::endl;

}*/

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ProcessCursorMovingGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
if(event == vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent){
  //std::cout<<"cursor's position: "<< this->VisuHisto->GetParameterCursorPosition() <<std::endl;
  
         // Build Filter
    this->ImageMeanIntensityPreNormalization =
      vtkImageMeanIntensityPreNormalization::New();
  
    this->ImageMeanIntensityPreNormalization->SetNormType
      (INTENSITY_NORM_MEAN_MRI);
    this->ImageMeanIntensityPreNormalization->SetNormValue
      (80);   
    this->ImageMeanIntensityPreNormalization->SetInitialHistogramSmoothingWidth
      (this->VisuHisto->GetParameterCursorPosition());
    this->ImageMeanIntensityPreNormalization->SetMaxHistogramSmoothingWidth
      (10);
    this->ImageMeanIntensityPreNormalization->SetRelativeMaxVoxelNum
      (0.99);
    this->ImageMeanIntensityPreNormalization->SetPrintInfo
      (1);
      
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  //int vol_id = mrmlManager->GetVolumeNthID(id_event);

  vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(id_event);
  
  vtkImageData* inputImage = volumeNode->GetImageData();
      
     std::cout<<"Volume: "<< mrmlManager->GetVolumeName(id_event)<<std::endl;
      
    this->ImageMeanIntensityPreNormalization->SetInput(inputImage);

    // execute filter

    this->ImageMeanIntensityPreNormalization->Update();
    
    //std::cout<<"test: "<<this->ImageMeanIntensityPreNormalization->GetInitialHistogramSmoothingWidth()<<std::endl;
    
    this->NormalizationValueRecommandedEntry->GetWidget()->SetValueAsDouble(this->ImageMeanIntensityPreNormalization->GetInitialHistogramSmoothingWidth());
    
    this->ImageMeanIntensityPreNormalization->Delete();
 } 
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::AddCursorMovingGUIEvents() 
{

    this->VisuHisto
    ->AddObserver(vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::RemoveCursorMovingGUIEvents()
{
//this->VisuHisto->RemoveObserver(vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent, this->GetGUI()->GetGUICallbackCommand());
}

