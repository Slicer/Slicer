/*=auto==============================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentNodeParametersStep.cxx,v$
  Date:      $Date: 2009/06/23 10:30:00$
  Version:   $Revision: 0.0$
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#include "vtkEMSegmentNodeParametersStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnListWithScrollbarsWithLabel.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWNotebook.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

#include "vtkKWHistogram.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkColorTransferFunction.h"
#include "vtkKWParameterValueFunctionInterface.h"
#include "vtkKWParameterValueFunctionEditor.h"

#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkPointData.h"

#include "vtkEMSegmentAnatomicalStructureStep.h"

#define EMSEG_PRINT_FREQUENCY_MAX 20

//-------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentNodeParametersStep);
vtkCxxRevisionMacro(vtkEMSegmentNodeParametersStep,"$Revision: 0.0$");

//-------------------------------------------------------------------
vtkEMSegmentNodeParametersStep::vtkEMSegmentNodeParametersStep()
{
  this->SetName("7/9. Edit Node-based Parameters");
  this->SetDescription("Specify node-based segmentation parameters");

  this->NodeParametersNotebook                 = NULL;
  this->NodeParametersGlobalPriorScale         = NULL;
  this->NodeParametersSpatialPriorWeightScale  = NULL;
  this->NodeParametersInputChannelWeightsList  = NULL;
  this->NodeParametersAlphaScale               = NULL;
  this->StoppingConditionsEMMenuButton         = NULL;
  this->StoppingConditionsEMIterationsEntry    = NULL;
  this->StoppingConditionsEMValueEntry         = NULL;
  this->StoppingConditionsMFAMenuButton        = NULL;
  this->StoppingConditionsMFAIterationsEntry   = NULL;
  this->StoppingConditionsMFAValueEntry        = NULL;
  this->StoppingConditionsBiasIterationsEntry  = NULL;
  this->PrintBasicFrame                        = NULL;
  this->NodeParametersPrintWeightCheckButton   = NULL;
  this->NodeParametersPrintQualityCheckButton  = NULL;
  this->NodeParametersPrintFrequencyScale      = NULL;
  this->NodeParametersPrintBiasCheckButton     = NULL;
  this->NodeParametersPrintLabelMapCheckButton = NULL;
  this->PrintConvergenceFrame                  = NULL;

  this->NodeParametersInteractionMatricesFrame = NULL;
  this->NodeParametersPCAFrame                 = NULL;
  this->NodeParametersRegistrationFrame        = NULL;
  this->NodeParametersMiscellaeneousFrame      = NULL;
  this->NodeParametersInhomogeneityFrame       = NULL;


  this->NumberOfClassesEntryLabel              = NULL;
  this->ClassAndNodeList                       = NULL;
  this->TestButton                             = NULL;

  this->NodeParametersPrintEMLabelMapConvergenceCheckButton    = NULL;
  this->NodeParametersPrintEMWeightsConvergenceCheckButton     = NULL;
  this->NodeParametersPrintMFALabelMapConvergenceCheckButton   = NULL;
  this->NodeParametersPrintMFAWeightsConvergenceCheckButton    = NULL;

  this->NodeParametersExcludeIncompleteEStepCheckButton        = NULL;
  this->NodeParametersGenerateBackgroundProbabilityCheckButton = NULL;
  this->IntensityDistributionHistogramButton                   = NULL;
  this->IntensityDistributionHistogramHistogram                = NULL;
  this->IntensityDistributionHistogramHistogramVisualization   = NULL;
}

//-------------------------------------------------------------------
vtkEMSegmentNodeParametersStep::~vtkEMSegmentNodeParametersStep()
{
  if (this->TestButton)
    {
    this->TestButton->Delete();
    this->TestButton = NULL;
    }

  if (this->ClassAndNodeList)
    {
    this->ClassAndNodeList->Delete();
    this->ClassAndNodeList = NULL;
    }

  if (this->NumberOfClassesEntryLabel)
    {
    this->NumberOfClassesEntryLabel->Delete();
    this->NumberOfClassesEntryLabel = NULL;
    }

  if (this->IntensityDistributionHistogramHistogramVisualization)
    {
    this->IntensityDistributionHistogramHistogramVisualization->Delete();
    this->IntensityDistributionHistogramHistogramVisualization = NULL;
    }

  if (this->IntensityDistributionHistogramHistogram)
    {
    this->IntensityDistributionHistogramHistogram->Delete();
    this->IntensityDistributionHistogramHistogram = NULL;
    }

  if (this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton->Delete();
    this->IntensityDistributionHistogramButton = NULL;
    }

  if (this->NodeParametersNotebook)
    {
    this->NodeParametersNotebook->Delete();
    this->NodeParametersNotebook = NULL;
    }

  if (this->NodeParametersGlobalPriorScale)
    {
    this->NodeParametersGlobalPriorScale->Delete();
    this->NodeParametersGlobalPriorScale = NULL;
    }

  if (this->NodeParametersSpatialPriorWeightScale)
    {
    this->NodeParametersSpatialPriorWeightScale->Delete();
    this->NodeParametersSpatialPriorWeightScale = NULL;
    }

  if (this->NodeParametersInputChannelWeightsList)
    {
    this->NodeParametersInputChannelWeightsList->Delete();
    this->NodeParametersInputChannelWeightsList = NULL;
    }

  if (this->NodeParametersAlphaScale)
    {
    this->NodeParametersAlphaScale->Delete();
    this->NodeParametersAlphaScale = NULL;
    }

  if (this->StoppingConditionsEMMenuButton)
    {
    this->StoppingConditionsEMMenuButton->Delete();
    this->StoppingConditionsEMMenuButton = NULL;
    }

  if (this->StoppingConditionsEMIterationsEntry)
    {
    this->StoppingConditionsEMIterationsEntry->Delete();
    this->StoppingConditionsEMIterationsEntry = NULL;
    }

  if (this->StoppingConditionsEMValueEntry)
    {
    this->StoppingConditionsEMValueEntry->Delete();
    this->StoppingConditionsEMValueEntry = NULL;
    }

  if (this->StoppingConditionsMFAMenuButton)
    {
    this->StoppingConditionsMFAMenuButton->Delete();
    this->StoppingConditionsMFAMenuButton = NULL;
    }

  if (this->StoppingConditionsMFAIterationsEntry)
    {
    this->StoppingConditionsMFAIterationsEntry->Delete();
    this->StoppingConditionsMFAIterationsEntry = NULL;
    }

  if (this->StoppingConditionsMFAValueEntry)
    {
    this->StoppingConditionsMFAValueEntry->Delete();
    this->StoppingConditionsMFAValueEntry = NULL;
    }

  if (this->StoppingConditionsBiasIterationsEntry)
    {
    this->StoppingConditionsBiasIterationsEntry->Delete();
    this->StoppingConditionsBiasIterationsEntry = NULL;
    }

  if (this->NodeParametersPrintWeightCheckButton)
    {
    this->NodeParametersPrintWeightCheckButton->Delete();
    this->NodeParametersPrintWeightCheckButton = NULL;
    }

  if (this->NodeParametersPrintQualityCheckButton)
    {
    this->NodeParametersPrintQualityCheckButton->Delete();
    this->NodeParametersPrintQualityCheckButton = NULL;
    }

  if (this->NodeParametersPrintFrequencyScale)
    {
    this->NodeParametersPrintFrequencyScale->Delete();
    this->NodeParametersPrintFrequencyScale = NULL;
    }

  if (this->NodeParametersPrintBiasCheckButton)
    {
    this->NodeParametersPrintBiasCheckButton->Delete();
    this->NodeParametersPrintBiasCheckButton = NULL;
    }

  if (this->NodeParametersPrintLabelMapCheckButton)
    {
    this->NodeParametersPrintLabelMapCheckButton->Delete();
    this->NodeParametersPrintLabelMapCheckButton = NULL;
    }

  if (this->PrintBasicFrame)
    {
    this->PrintBasicFrame->Delete();
    this->PrintBasicFrame = NULL;
    }

  if (this->NodeParametersPrintEMLabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton->Delete();
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton = NULL;
    }

  if (this->NodeParametersPrintEMWeightsConvergenceCheckButton)
    {
    this->NodeParametersPrintEMWeightsConvergenceCheckButton->Delete();
    this->NodeParametersPrintEMWeightsConvergenceCheckButton = NULL;
    }

  if (this->NodeParametersPrintMFALabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->Delete();
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton = NULL;
    }

  if (this->NodeParametersPrintMFAWeightsConvergenceCheckButton)
    {
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton->Delete();
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton = NULL;
    }

  if (this->PrintConvergenceFrame)
    {
    this->PrintConvergenceFrame->Delete();
    this->PrintConvergenceFrame = NULL;
    }

  if (this->NodeParametersInteractionMatricesFrame)
    {
    this->NodeParametersInteractionMatricesFrame->Delete();
    this->NodeParametersInteractionMatricesFrame = NULL;
    }

  if (this->NodeParametersPCAFrame)
    {
    this->NodeParametersPCAFrame->Delete();
    this->NodeParametersPCAFrame = NULL;
    }

  if (this->NodeParametersRegistrationFrame)
    {
    this->NodeParametersRegistrationFrame->Delete();
    this->NodeParametersRegistrationFrame = NULL;
    }

  if (this->NodeParametersExcludeIncompleteEStepCheckButton)
    {
    this->NodeParametersExcludeIncompleteEStepCheckButton->Delete();
    this->NodeParametersExcludeIncompleteEStepCheckButton = NULL;
    }

  if (this->NodeParametersGenerateBackgroundProbabilityCheckButton)
    {
    this->NodeParametersGenerateBackgroundProbabilityCheckButton->Delete();
    this->NodeParametersGenerateBackgroundProbabilityCheckButton = NULL;
    }

  if (this->NodeParametersMiscellaeneousFrame)
    {
    this->NodeParametersMiscellaeneousFrame->Delete();
    this->NodeParametersMiscellaeneousFrame = NULL;
    }

  if (this->NodeParametersInhomogeneityFrame)
    {
    this->NodeParametersInhomogeneityFrame->Delete();
    this->NodeParametersInhomogeneityFrame = NULL;
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anat_step =
    this->GetGUI()->GetAnatomicalStructureStep();
  anat_step->ShowAnatomicalStructureTree();

  vtkKWWidget *parent = wizard_widget->GetClientArea();;

  // Override the tree callbacks for that specific step

  anat_step->GetAnatomicalStructureTree()->GetWidget()->
    SetSelectionChangedCommand(this,"DisplaySelectedNodeParametersCallback");

  // Create the notebook

  if (!this->NodeParametersNotebook)
    {
    this->NodeParametersNotebook = vtkKWNotebook::New();
    }
  if (!this->NodeParametersNotebook->IsCreated())
    {
    this->NodeParametersNotebook->SetParent(parent);
    this->NodeParametersNotebook->Create();
    this->NodeParametersNotebook->AddPage("Priors");
    this->NodeParametersNotebook->AddPage("Basic");
    this->NodeParametersNotebook->AddPage("Stopping Conditions");
    this->NodeParametersNotebook->AddPage("Print");
    this->NodeParametersNotebook->AddPage("Advanced");
    }

  vtkKWFrame *prior_page =
    this->NodeParametersNotebook->GetFrame("Priors");
  vtkKWFrame *basic_page =
    this->NodeParametersNotebook->GetFrame("Basic");
  vtkKWFrame *stop_cond_page =
    this->NodeParametersNotebook->GetFrame("Stopping Conditions");
  vtkKWFrame *print_page =
    this->NodeParametersNotebook->GetFrame("Print");
  vtkKWFrame *advanced_page =
    this->NodeParametersNotebook->GetFrame("Advanced");

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 2 -pady 2",
    this->NodeParametersNotebook->GetWidgetName());

  // Create the class probability scale

  std::string msg = "Probability that a voxel belonging to the parent ";
  msg += "structure will also belong to this structure.  The value must be ";
  msg += "in the range [0,1].  Global priors for each set of siblings must ";
  msg += "sum to 1.";

  if (!this->NodeParametersGlobalPriorScale)
    {
    this->NodeParametersGlobalPriorScale = vtkKWScaleWithEntry::New();
    }
  if (!this->NodeParametersGlobalPriorScale->IsCreated())
    {
    this->NodeParametersGlobalPriorScale->SetParent(basic_page);
    this->NodeParametersGlobalPriorScale->PopupModeOn();
    this->NodeParametersGlobalPriorScale->Create();
    this->NodeParametersGlobalPriorScale->SetEntryWidth(4);
    this->NodeParametersGlobalPriorScale->SetLabelText("Global Prior:");
    this->NodeParametersGlobalPriorScale->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 9);
    this->NodeParametersGlobalPriorScale->SetRange(0.0, 1.0);
    this->NodeParametersGlobalPriorScale->SetResolution(0.01);
    this->NodeParametersGlobalPriorScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    this->NodeParametersGlobalPriorScale->SetBalloonHelpString(msg.c_str());
    }

  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2",
    this->NodeParametersGlobalPriorScale->GetWidgetName());

  // Create the spatial prior weight scale

  msg = "Weight of the atlas (spatial prior) in the segmentation decision.  ";
  msg += "The value must be in the range [0,1], where 0 indicates that the ";
  msg += "atlas is ignored and 1 indicates the maximum atlas weight.";

  if (!this->NodeParametersSpatialPriorWeightScale)
    {
    this->NodeParametersSpatialPriorWeightScale = vtkKWScaleWithEntry
      ::New();
    }
  if (!this->NodeParametersSpatialPriorWeightScale->IsCreated())
    {
    this->NodeParametersSpatialPriorWeightScale->SetParent(basic_page);
    this->NodeParametersSpatialPriorWeightScale->PopupModeOn();
    this->NodeParametersSpatialPriorWeightScale->Create();
    this->NodeParametersSpatialPriorWeightScale->SetEntryWidth(4);
    this->NodeParametersSpatialPriorWeightScale->SetLabelText(
        "Atlas Weight:");
    this->NodeParametersSpatialPriorWeightScale->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH - 9);
    this->NodeParametersSpatialPriorWeightScale->SetRange(0.0, 1.0);
    this->NodeParametersSpatialPriorWeightScale->SetResolution(0.01);
    this->NodeParametersSpatialPriorWeightScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    this->NodeParametersSpatialPriorWeightScale->SetBalloonHelpString(
        msg.c_str());
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2",
    this->NodeParametersSpatialPriorWeightScale->GetWidgetName());

  // Create the input channel weights list

  if (!this->NodeParametersInputChannelWeightsList)
    {
    this->NodeParametersInputChannelWeightsList =
      vtkKWMultiColumnListWithScrollbarsWithLabel::New();
    }
  if (!this->NodeParametersInputChannelWeightsList->IsCreated())
    {
    this->NodeParametersInputChannelWeightsList->SetParent(basic_page);
    this->NodeParametersInputChannelWeightsList->Create();
    this->NodeParametersInputChannelWeightsList->SetLabelText(
      "Input Channel Weights:");
    this->NodeParametersInputChannelWeightsList->SetLabelPositionToTop();
    this->NodeParametersInputChannelWeightsList->GetWidget()->
      HorizontalScrollbarVisibilityOff();

    vtkKWMultiColumnList *list = this->NodeParametersInputChannelWeightsList->
      GetWidget()->GetWidget();

    list->SetHeight(4);
    list->MovableColumnsOff();
    list->SetSelectionModeToSingle();
    list->ResizableColumnsOff();

    int col_id = list->AddColumn("Volume");
    list->SetColumnWidth(col_id, 15);
    list->SetColumnEditable(col_id, 0);
    col_id = list->AddColumn("Weight");
    list->SetColumnEditable(col_id, 1);

    msg = "Weight of each channel in the segmentation decision.  Right-click";
    msg += " or double-click to modify weights.  Weights should be in the ";
    msg += "range [0,1]; 0 indicates that the channel is ignored and 1 ";
    msg += "indicates the maximum channel weight.  The weights are not ";
    msg += "dependent on each other or any other weights.";

    list->SetRightClickCommand
      (this, "RightClickOnInputChannelWeightsListCallback");
    list->SetBalloonHelpString(msg.c_str());
    }

  this->Script(
      "grid %s -column 1 -row 0 -rowspan 3 -sticky news -padx 2 -pady 2",
      this->NodeParametersInputChannelWeightsList->GetWidgetName());

  // Create the alpha scale

  if (!this->NodeParametersAlphaScale)
    {
    this->NodeParametersAlphaScale = vtkKWScaleWithEntry::New();
    }
  if (!this->NodeParametersAlphaScale->IsCreated())
    {
    this->NodeParametersAlphaScale->SetParent(basic_page);
    this->NodeParametersAlphaScale->PopupModeOn();
    this->NodeParametersAlphaScale->Create();
    this->NodeParametersAlphaScale->SetEntryWidth(4);
    this->NodeParametersAlphaScale->SetLabelText("Alpha:");
    this->NodeParametersAlphaScale->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 9);
    this->NodeParametersAlphaScale->SetRange(0.0, 1.0);
    this->NodeParametersAlphaScale->SetResolution(0.01);
    this->NodeParametersAlphaScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 2 -sticky nw -padx 2 -pady 2",
               this->NodeParametersAlphaScale->GetWidgetName());

  this->Script("grid columnconfigure %s 1 -weight 1", basic_page->
      GetWidgetName());

  // Get the number of leaves

  this->NumberOfLeaves = 0;
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL,root_id);
      }
    }

  // Create the histogram volume selector

  if (!this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->IntensityDistributionHistogramButton->IsCreated())
    {
    this->IntensityDistributionHistogramButton->SetParent(prior_page);
    this->IntensityDistributionHistogramButton->Create();
    this->IntensityDistributionHistogramButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH+10);
    this->IntensityDistributionHistogramButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->IntensityDistributionHistogramButton->SetLabelText("Target Image:");
    this->IntensityDistributionHistogramButton->SetBalloonHelpString(
        "Select a target image to adjust intensity distribution");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
    this->IntensityDistributionHistogramButton->GetWidgetName());

  this->PopulateIntensityDistributionTargetVolumeSelector();

  // Create the histogram

  if (!this->IntensityDistributionHistogramHistogram)
    {
    this->IntensityDistributionHistogramHistogram = vtkKWHistogram::New();
    this->IntensityDistributionHistogramHistogramVisualization =
      vtkKWColorTransferFunctionEditor::New();
    this->IntensityDistributionHistogramHistogramFunc =
      vtkColorTransferFunction::New();
    }

   if (!this->IntensityDistributionHistogramHistogramVisualization->
       IsCreated())
    {
    this->IntensityDistributionHistogramHistogramVisualization->SetParent(
      prior_page);
    this->IntensityDistributionHistogramHistogramVisualization->Create();
    this->IntensityDistributionHistogramHistogramVisualization->
      SetBorderWidth(2);
    this->IntensityDistributionHistogramHistogramVisualization->
      SetReliefToGroove();
    this->IntensityDistributionHistogramHistogramVisualization->SetPadX(2);
    this->IntensityDistributionHistogramHistogramVisualization->SetPadY(2);
    this->IntensityDistributionHistogramHistogramVisualization->
      SetPointPositionInValueRangeToTop();
    this->IntensityDistributionHistogramHistogramVisualization->
      MidPointEntryVisibilityOn();
    this->IntensityDistributionHistogramHistogramVisualization->
      MidPointGuidelineVisibilityOn();
    this->IntensityDistributionHistogramHistogramVisualization->
      SetLabelPositionToTop();
    }

   this->Script(
       "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
       this->IntensityDistributionHistogramHistogramVisualization->
       GetWidgetName());

    vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
    this->IntensityDistributionHistogramButton->SetEnabled(
    mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->IntensityDistributionHistogramButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int vol_id = mrmlManager->GetVolumeNthID(0);
    this->IntensityDistributionTargetSelectionChangedCallback(vol_id);
    }

  this->AddPointMovingGUIEvents();
  this->AddPointAddGUIEvents();

  // List Creation

  if (!this->ClassAndNodeList)
    {
    this->ClassAndNodeList = vtkKWMultiColumnList::New();
    }

  if (!this->ClassAndNodeList->IsCreated())
    {
    this->ClassAndNodeList->SetParent(prior_page);
    this->ClassAndNodeList->Create();
    this->ClassAndNodeList->MovableColumnsOff();
    this->ClassAndNodeList->SetPotentialCellColorsChangedCommand(
      this->ClassAndNodeList,
      "ScheduleRefreshColorsOfAllCellsWithWindowCommand");

    int col_index = this->ClassAndNodeList->AddColumn("Class");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);

    col_index = this->ClassAndNodeList->AddColumn("Point");
    this->ClassAndNodeList->ColumnEditableOff(col_index);
    this->ClassAndNodeList->SetColumnAlignmentToCenter(col_index);

    col_index = this->ClassAndNodeList->AddColumn("Color");
    this->ClassAndNodeList->ColumnEditableOff(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);
    this->ClassAndNodeList->SetColumnAlignmentToCenter(col_index);
    }

  const char* maintin[200];
  double nodeValue[6];

  for(int j = 0;j < this->NumberOfLeaves;j++)
    {
    maintin[j] = mrmlManager->GetTreeNodeName(this->LeafId[j]);
    }

  for(int i=0; i < this->NumberOfLeaves; i++)
    {
    this->ClassAndNodeList->InsertCellText(i,0,maintin[i]);
    this->ClassAndNodeList->SetCellWindowCommandToComboBoxWithValues(
        i,0,this->NumberOfLeaves,maintin);
    this->ClassAndNodeList->InsertCellTextAsInt(i,1,i+1);

    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i,
        nodeValue);
    this->ClassAndNodeList->SetCellWindowCommandToColorButton(i,2);
  }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2 -pady 2",
    this->ClassAndNodeList->GetWidgetName());

  // Update Button

  if (!this->TestButton)
    {
    this->TestButton = vtkKWPushButton::New();
    }
  if (!this->TestButton->IsCreated())
    {
    this->TestButton->SetParent(prior_page);
    this->TestButton->Create();
    this->TestButton->SetText("Update Tree");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
    this->TestButton->GetWidgetName());

  this->AddTestButtonGUIEvents();

  // Create the EM menu button

  if (!this->StoppingConditionsEMMenuButton)
    {
    this->StoppingConditionsEMMenuButton = vtkKWMenuButtonWithLabel::
      New();
    }
  if (!this->StoppingConditionsEMMenuButton->IsCreated())
    {
    this->StoppingConditionsEMMenuButton->SetParent(stop_cond_page);
    this->StoppingConditionsEMMenuButton->Create();
    this->StoppingConditionsEMMenuButton->SetLabelText("EM:");
    this->StoppingConditionsEMMenuButton->SetLabelWidth(5);
    this->StoppingConditionsEMMenuButton->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH);
    }

  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsEMMenuButton->GetWidgetName());

  // Create the EM iterations entry

  if (!this->StoppingConditionsEMIterationsEntry)
    {
    this->StoppingConditionsEMIterationsEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->StoppingConditionsEMIterationsEntry->IsCreated())
    {
    this->StoppingConditionsEMIterationsEntry->SetParent(stop_cond_page);
    this->StoppingConditionsEMIterationsEntry->Create();
    this->StoppingConditionsEMIterationsEntry->SetLabelText("Iterations:");
    this->StoppingConditionsEMIterationsEntry->SetLabelWidth(15);

    vtkKWEntry *entry = this->StoppingConditionsEMIterationsEntry->
      GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2",
    this->StoppingConditionsEMIterationsEntry->GetWidgetName());

  // Create the EM value entry

  if (!this->StoppingConditionsEMValueEntry)
    {
    this->StoppingConditionsEMValueEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->StoppingConditionsEMValueEntry->IsCreated())
    {
    this->StoppingConditionsEMValueEntry->SetParent(stop_cond_page);
    this->StoppingConditionsEMValueEntry->Create();
    this->StoppingConditionsEMValueEntry->SetLabelText("Value:");
    this->StoppingConditionsEMValueEntry->SetLabelWidth(15);
    vtkKWEntry *entry = this->StoppingConditionsEMValueEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToDouble();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsEMValueEntry->GetWidgetName());

  // Create the MFA menubutton

  if (!this->StoppingConditionsMFAMenuButton)
    {
    this->StoppingConditionsMFAMenuButton = vtkKWMenuButtonWithLabel
      ::New();
    }
  if (!this->StoppingConditionsMFAMenuButton->IsCreated())
    {
    this->StoppingConditionsMFAMenuButton->SetParent(stop_cond_page);
    this->StoppingConditionsMFAMenuButton->Create();
    this->StoppingConditionsMFAMenuButton->SetLabelText("MFA:");
    this->StoppingConditionsMFAMenuButton->SetLabelWidth(5);
    this->StoppingConditionsMFAMenuButton->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH);
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsMFAMenuButton->GetWidgetName());

  // Create the MFA iterations entry

  if (!this->StoppingConditionsMFAIterationsEntry)
    {
    this->StoppingConditionsMFAIterationsEntry = vtkKWEntryWithLabel
      ::New();
    }
  if (!this->StoppingConditionsMFAIterationsEntry->IsCreated())
    {
    this->StoppingConditionsMFAIterationsEntry->SetParent(stop_cond_page);
    this->StoppingConditionsMFAIterationsEntry->Create();
    this->StoppingConditionsMFAIterationsEntry->SetLabelText("Iterations:");
    this->StoppingConditionsMFAIterationsEntry->SetLabelWidth(15);
    vtkKWEntry *entry =
      this->StoppingConditionsMFAIterationsEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 1 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsMFAIterationsEntry->GetWidgetName());

  // Create the MFA value entry

  if (!this->StoppingConditionsMFAValueEntry)
    {
    this->StoppingConditionsMFAValueEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->StoppingConditionsMFAValueEntry->IsCreated())
    {
    this->StoppingConditionsMFAValueEntry->SetParent(stop_cond_page);
    this->StoppingConditionsMFAValueEntry->Create();
    this->StoppingConditionsMFAValueEntry->SetLabelText("Value:");
    this->StoppingConditionsMFAValueEntry->SetLabelWidth(15);
    vtkKWEntry *entry = this->StoppingConditionsMFAValueEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToDouble();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 1 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsMFAValueEntry->GetWidgetName());

  // Create the Bias calculation max iterations entry

  if (!this->StoppingConditionsBiasIterationsEntry)
    {
    this->StoppingConditionsBiasIterationsEntry = vtkKWEntryWithLabel
      ::New();
    }
  if (!this->StoppingConditionsBiasIterationsEntry->IsCreated())
    {
    this->StoppingConditionsBiasIterationsEntry->SetParent(stop_cond_page);
    this->StoppingConditionsBiasIterationsEntry->Create();
    this->StoppingConditionsBiasIterationsEntry->SetLabelText(
      "Bias Iterations:");
    this->StoppingConditionsBiasIterationsEntry->SetLabelWidth(15);
    vtkKWEntry *entry =
      this->StoppingConditionsBiasIterationsEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 1 -row 2 -sticky nw -padx 2 -pady 2",
      this->StoppingConditionsBiasIterationsEntry->GetWidgetName());

  this->Script("grid columnconfigure %s 0 -weight 1",
      stop_cond_page->GetWidgetName());
  this->Script("grid columnconfigure %s 1 -weight 1",
      stop_cond_page->GetWidgetName());

  // Create the print basic frame

  if (!this->PrintBasicFrame)
    {
    this->PrintBasicFrame = vtkKWFrame::New();
    }
  if (!this->PrintBasicFrame->IsCreated())
    {
    this->PrintBasicFrame->SetParent(print_page);
    this->PrintBasicFrame->Create();
    }

  this->Script("pack %s -side left -anchor nw -padx 5 -pady {20 2}",
    this->PrintBasicFrame->GetWidgetName());

  // Create the print weight checkbutton

  if (!this->NodeParametersPrintWeightCheckButton)
    {
    this->NodeParametersPrintWeightCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintWeightCheckButton->IsCreated())
    {
    this->NodeParametersPrintWeightCheckButton->SetParent(this->
        PrintBasicFrame);
    this->NodeParametersPrintWeightCheckButton->Create();
    this->NodeParametersPrintWeightCheckButton->SetLabelText("Weight:");
    this->NodeParametersPrintWeightCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH-10);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
      this->NodeParametersPrintWeightCheckButton->GetWidgetName());

  // Create the print quality checkbutton

  if (!this->NodeParametersPrintQualityCheckButton)
    {
    this->NodeParametersPrintQualityCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintQualityCheckButton->IsCreated())
    {
    this->NodeParametersPrintQualityCheckButton->SetParent(this->
        PrintBasicFrame);
    this->NodeParametersPrintQualityCheckButton->Create();
    this->NodeParametersPrintQualityCheckButton->SetLabelText("Quality:");
    this->NodeParametersPrintQualityCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH-10);
    }

  // Create the print frequency scale

  if (!this->NodeParametersPrintFrequencyScale)
    {
    this->NodeParametersPrintFrequencyScale = vtkKWScaleWithEntry::New();
    }
  if (!this->NodeParametersPrintFrequencyScale->IsCreated())
    {
    this->NodeParametersPrintFrequencyScale->SetParent(this->PrintBasicFrame);
    this->NodeParametersPrintFrequencyScale->PopupModeOn();
    this->NodeParametersPrintFrequencyScale->Create();
    this->NodeParametersPrintFrequencyScale->SetEntryWidth(4);
    this->NodeParametersPrintFrequencyScale->GetEntry()->
      SetRestrictValueToInteger();
    this->NodeParametersPrintFrequencyScale->SetLabelText(
      "Frequency:");
    this->NodeParametersPrintFrequencyScale->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->NodeParametersPrintFrequencyScale->SetRange(0,
        EMSEG_PRINT_FREQUENCY_MAX);
    this->NodeParametersPrintFrequencyScale->SetResolution(1);
    this->NodeParametersPrintFrequencyScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    }

  // Create the print bias checkbutton

  if (!this->NodeParametersPrintBiasCheckButton)
    {
    this->NodeParametersPrintBiasCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintBiasCheckButton->IsCreated())
    {
    this->NodeParametersPrintBiasCheckButton->SetParent(this->
        PrintBasicFrame);
    this->NodeParametersPrintBiasCheckButton->Create();
    this->NodeParametersPrintBiasCheckButton->SetLabelText("Bias:");
    this->NodeParametersPrintBiasCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH-10);
    }

  // Create the print label map checkbutton

  if (!this->NodeParametersPrintLabelMapCheckButton)
    {
    this->NodeParametersPrintLabelMapCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintLabelMapCheckButton->IsCreated())
    {
    this->NodeParametersPrintLabelMapCheckButton->SetParent(
        this->PrintBasicFrame);
    this->NodeParametersPrintLabelMapCheckButton->Create();
    this->NodeParametersPrintLabelMapCheckButton->SetLabelText("Label Map:");
    this->NodeParametersPrintLabelMapCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH-10);
    }

  // Create the print convergence frame

  if (!this->PrintConvergenceFrame)
    {
    this->PrintConvergenceFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->PrintConvergenceFrame->IsCreated())
    {
    this->PrintConvergenceFrame->SetParent(print_page);
    this->PrintConvergenceFrame->Create();
    this->PrintConvergenceFrame->SetLabelText("Convergence");
    }

  // Create the print EM label map convergence checkbutton

  if (!this->NodeParametersPrintEMLabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintEMLabelMapConvergenceCheckButton->IsCreated())
    {
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton->SetParent(
      this->PrintConvergenceFrame->GetFrame());
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton->Create();
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton->
      SetLabelText("EM Label Map:");
    this->NodeParametersPrintEMLabelMapConvergenceCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1",this->
      NodeParametersPrintEMLabelMapConvergenceCheckButton->GetWidgetName());

  // Create the print EM weights convergence checkbutton

  if (!this->NodeParametersPrintEMWeightsConvergenceCheckButton)
    {
    this->NodeParametersPrintEMWeightsConvergenceCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintEMWeightsConvergenceCheckButton->IsCreated())
    {
    this->NodeParametersPrintEMWeightsConvergenceCheckButton->SetParent(
      this->PrintConvergenceFrame->GetFrame());
    this->NodeParametersPrintEMWeightsConvergenceCheckButton->Create();
    this->NodeParametersPrintEMWeightsConvergenceCheckButton->SetLabelText(
      "EM Weights:");
    this->NodeParametersPrintEMWeightsConvergenceCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1",this->
      NodeParametersPrintEMWeightsConvergenceCheckButton->GetWidgetName());

  // Create the print MFA label map convergence checkbutton

  if (!this->NodeParametersPrintMFALabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintMFALabelMapConvergenceCheckButton->
      IsCreated())
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetParent(
      this->PrintConvergenceFrame->GetFrame());
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->Create();
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetLabelText(
      "MFA Label Map:");
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1",this->
      NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidgetName());

  // Create the print MFA weights convergence checkbutton

  if (!this->NodeParametersPrintMFAWeightsConvergenceCheckButton)
    {
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintMFAWeightsConvergenceCheckButton->IsCreated())
    {
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton->SetParent(
      this->PrintConvergenceFrame->GetFrame());
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton->Create();
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton->SetLabelText(
      "MFA Weights:");
    this->NodeParametersPrintMFAWeightsConvergenceCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1",this->
      NodeParametersPrintMFAWeightsConvergenceCheckButton->GetWidgetName());

  // Create the frame

  if (!this->NodeParametersPCAFrame)
    {
    this->NodeParametersPCAFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersPCAFrame->IsCreated())
    {
    this->NodeParametersPCAFrame->SetParent(advanced_page);
    this->NodeParametersPCAFrame->Create();
    this->NodeParametersPCAFrame->SetLabelText("PCA Parameters");
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    this->NodeParametersPCAFrame->GetWidgetName());

  // Create the frame

  if (!this->NodeParametersRegistrationFrame)
    {
    this->NodeParametersRegistrationFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersRegistrationFrame->IsCreated())
    {
    this->NodeParametersRegistrationFrame->SetParent(advanced_page);
    this->NodeParametersRegistrationFrame->Create();
    this->NodeParametersRegistrationFrame->SetLabelText(
      "Registration Parameters");
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    this->NodeParametersRegistrationFrame->GetWidgetName());

  // Create the frame

  if (!this->NodeParametersInteractionMatricesFrame)
    {
    this->NodeParametersInteractionMatricesFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersInteractionMatricesFrame->IsCreated())
    {
    this->NodeParametersInteractionMatricesFrame->SetParent(advanced_page);
    this->NodeParametersInteractionMatricesFrame->Create();
    this->NodeParametersInteractionMatricesFrame->SetLabelText(
      "Class Interaction Matrices");
    }

  // Create the frame

  if (!this->NodeParametersInhomogeneityFrame)
    {
    this->NodeParametersInhomogeneityFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersInhomogeneityFrame->IsCreated())
    {
    this->NodeParametersInhomogeneityFrame->SetParent(advanced_page);
    this->NodeParametersInhomogeneityFrame->Create();
    this->NodeParametersInhomogeneityFrame->SetLabelText(
      "Inhomogeneity Parameters");
    }

  // Create the miscellaeneous parameters frame

  if (!this->NodeParametersMiscellaeneousFrame)
    {
    this->NodeParametersMiscellaeneousFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersMiscellaeneousFrame->IsCreated())
    {
    this->NodeParametersMiscellaeneousFrame->SetParent(advanced_page);
    this->NodeParametersMiscellaeneousFrame->Create();
    this->NodeParametersMiscellaeneousFrame->SetLabelText(
      "Miscellaeneous Parameters");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
    this->NodeParametersMiscellaeneousFrame->GetWidgetName());

  // Create the Tree node exclude from incomplete EStep check button

  if (!this->NodeParametersExcludeIncompleteEStepCheckButton)
    {
    this->NodeParametersExcludeIncompleteEStepCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersExcludeIncompleteEStepCheckButton->IsCreated())
    {
    this->NodeParametersExcludeIncompleteEStepCheckButton->SetParent(
      this->NodeParametersMiscellaeneousFrame->GetFrame());
    this->NodeParametersExcludeIncompleteEStepCheckButton->Create();
    this->NodeParametersExcludeIncompleteEStepCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->NodeParametersExcludeIncompleteEStepCheckButton->GetLabel()->
      SetText("Exclude From Incomplete EStep:");
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
    this->NodeParametersExcludeIncompleteEStepCheckButton->GetWidgetName());

  // Create the Tree node generate background probability check button

  if (!this->NodeParametersGenerateBackgroundProbabilityCheckButton)
    {
    this->NodeParametersGenerateBackgroundProbabilityCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersGenerateBackgroundProbabilityCheckButton->
      IsCreated())
    {
    this->NodeParametersGenerateBackgroundProbabilityCheckButton->SetParent(
      this->NodeParametersMiscellaeneousFrame->GetFrame());
    this->NodeParametersGenerateBackgroundProbabilityCheckButton->Create();
    this->NodeParametersGenerateBackgroundProbabilityCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->NodeParametersGenerateBackgroundProbabilityCheckButton->GetLabel()->
      SetText("Generate Background Probability:");
    }

  this->DisplaySelectedNodeParametersCallback();
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::PopulateClassAndNodeList()
{
  for(int i=0; i < this->NumberOfLeaves; i++)
  {
    this->ClassAndNodeList->InsertCellTextAsInt(i,1,i+1);
  }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  PopulateIntensityDistributionTargetVolumeSelector()
{
  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  int nb_of_target_volumes = mrmlManager->
    GetTargetNumberOfSelectedVolumes();

  vtkKWMenu* menu = this->IntensityDistributionHistogramButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d",
        "IntensityDistributionTargetSelectionChangedCallback",
        static_cast<int>(target_vol_id));
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  IntensityDistributionTargetSelectionChangedCallback(vtkIdType
    target_vol_id)
{
  // INTENSITY DISTRIBUTION TARGET SELECTION CHANGED CALLBACK

  vtkDataArray* array = this->GetGUI()->GetMRMLManager()->
    GetVolumeNode(target_vol_id)->GetImageData()->GetPointData()->
    GetScalars();

  this->IntensityDistributionHistogramHistogram->BuildHistogram(array,0);

  this->IntensityDistributionHistogramHistogramVisualization->SetHistogram(
    this->IntensityDistributionHistogramHistogram);

  double range[2];
  this->IntensityDistributionHistogramHistogram->GetRange(range);

  int size = this->IntensityDistributionHistogramHistogramVisualization->
    GetFunctionSize();

  this->IntensityDistributionHistogramHistogramVisualization->
    SetDisableAddAndRemove(0);

  if(size > 0)
    {
    this->IntensityDistributionHistogramHistogramFunc->RemoveAllPoints();
    }

  this->IntensityDistributionHistogramHistogramFunc->SetColorSpaceToHSV();
  this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[0],
      0.667, 1.0, 1.0);
  this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[1],
      0.000, 1.0, 1.0);

  if (this->NumberOfLeaves > 2)
    {
    double s, t, delta;
    double factor, color;

    delta = range[1] - range[0];
    factor = 1.0 / this->NumberOfLeaves;

    for(int i=1; i < this->NumberOfLeaves; i++)
      {
      t = i * factor; // [ 0.0, 1.0 ]
      s = t < 1.0 ? 1.0 - t : 0.0;

      color = 0.667 * s;

      this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(
        range[0] + t * delta, color, 1.0, 1.0);
      }
    }

  this->IntensityDistributionHistogramHistogramVisualization->
    SetDisableAddAndRemove(1);

  this->IntensityDistributionHistogramHistogramVisualization->
    SetColorTransferFunction(this->
      IntensityDistributionHistogramHistogramFunc);

  this->IntensityDistributionHistogramHistogramVisualization->
    SetWholeParameterRangeToFunctionRange();

  this->IntensityDistributionHistogramHistogramVisualization->
    SetVisibleParameterRangeToWholeParameterRange();

  this->IntensityDistributionHistogramHistogramVisualization->
    ParameterRangeVisibilityOn();

  this->IntensityDistributionHistogramHistogramVisualization->
    ExpandCanvasWidthOn();

  this->IntensityDistributionHistogramHistogramVisualization->
    SetCanvasHeight(180);
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::DisplaySelectedNodeParametersCallback()
{
  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkEMSegmentAnatomicalStructureStep *anat_step =
    this->GetGUI()->GetAnatomicalStructureStep();

  if (!mrmlManager || !anat_step)
    {
    return;
    }

  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id = 0;

  int sel_is_leaf_node = 0;
  int has_valid_selection = tree->HasSelection();

  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    sel_is_leaf_node = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    }

  int enabled = tree->GetEnabled();
  int row;
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];

  // Update the class probability scale

  if (this->NodeParametersGlobalPriorScale)
    {
    if (has_valid_selection)
      {
      this->NodeParametersGlobalPriorScale->SetEnabled(enabled);
      sprintf(buffer, "NodeParametersGlobalPriorChangedCallback %d",
        static_cast<int>(sel_vol_id));
      this->NodeParametersGlobalPriorScale->SetEndCommand(this, buffer);
      this->NodeParametersGlobalPriorScale->SetEntryCommand(this, buffer);
      this->NodeParametersGlobalPriorScale->SetValue(
        mrmlManager->GetTreeNodeClassProbability(sel_vol_id));
      }
    else
      {
      this->NodeParametersGlobalPriorScale->SetEnabled(0);
      this->NodeParametersGlobalPriorScale->SetEndCommand(NULL, NULL);
      this->NodeParametersGlobalPriorScale->SetEntryCommand(NULL, NULL);
      this->NodeParametersGlobalPriorScale->SetValue(0.0);
      }
    }

  // Update the spatial prior weight scale

  if (this->NodeParametersSpatialPriorWeightScale)
    {
    if (has_valid_selection)
      {
      this->NodeParametersSpatialPriorWeightScale->SetEnabled(enabled);
      sprintf(buffer,"NodeParametersSpatialPriorWeightChangedCallback %d",
          static_cast<int>(sel_vol_id));
      this->NodeParametersSpatialPriorWeightScale->SetEndCommand(
        this, buffer);
      this->NodeParametersSpatialPriorWeightScale->SetEntryCommand(
        this, buffer);
      this->NodeParametersSpatialPriorWeightScale->SetValue(
        mrmlManager->GetTreeNodeSpatialPriorWeight(sel_vol_id));
      }
    else
      {
      this->NodeParametersSpatialPriorWeightScale->SetEnabled(0);
      this->NodeParametersSpatialPriorWeightScale->SetEndCommand(NULL,NULL);
      this->NodeParametersSpatialPriorWeightScale->SetEntryCommand(NULL,NULL);
      this->NodeParametersSpatialPriorWeightScale->SetValue(0.0);
      }
    }

  // Update the input channel weights

  if (this->NodeParametersInputChannelWeightsList)
    {
    vtkKWMultiColumnList *list =
      this->NodeParametersInputChannelWeightsList->GetWidget()->GetWidget();
    list->DeleteAllRows();
    if (has_valid_selection)
      {
      this->NodeParametersInputChannelWeightsList->SetEnabled(enabled);
      sprintf(buffer,"NodeParametersInputChannelWeightChangedCallback %d",
          static_cast<int>(sel_vol_id));
      list->SetCellUpdatedCommand(this, buffer);
      for (row = 0; row < nb_of_target_volumes; row++)
        {
        list->AddRow();
        int vol_id = mrmlManager->GetTargetSelectedVolumeNthID(row);
        list->SetCellText(row, 0, mrmlManager->GetVolumeName(vol_id));
        list->SetCellTextAsDouble(row, 1,
          mrmlManager->GetTreeNodeInputChannelWeight(sel_vol_id, row));
        }
      }
    else
      {
      this->NodeParametersInputChannelWeightsList->SetEnabled(0);
      list->SetCellUpdatedCommand(NULL, NULL);
      }
    }

  // Update the alpha scale

  if (this->NodeParametersAlphaScale)
    {
    if (has_valid_selection)
      {
      this->NodeParametersAlphaScale->SetEnabled(enabled);
      sprintf(buffer, "NodeParametersAlphaChangedCallback %d",
          static_cast<int>(sel_vol_id));
      this->NodeParametersAlphaScale->SetEndCommand(this, buffer);
      this->NodeParametersAlphaScale->SetEntryCommand(this, buffer);
      this->NodeParametersAlphaScale->SetValue(
        mrmlManager->GetTreeNodeAlpha(sel_vol_id));
      }
    else
      {
      this->NodeParametersAlphaScale->SetEnabled(0);
      this->NodeParametersAlphaScale->SetEndCommand(NULL, NULL);
      this->NodeParametersAlphaScale->SetEntryCommand(NULL, NULL);
      this->NodeParametersAlphaScale->SetValue(0.0);
      }
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->Script("grid %s", this->NodeParametersAlphaScale->
          GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",
          this->NodeParametersAlphaScale->GetWidgetName());
      }
    }

  // Update the stopping conditions

  if (this->NodeParametersNotebook)
    {
    this->NodeParametersNotebook->SetPageEnabled(
      "Stopping Conditions", has_valid_selection && !sel_is_leaf_node);
    }

  // Update the EM menu button

  if (this->StoppingConditionsEMMenuButton)
    {
    vtkKWMenu *menu =
      this->StoppingConditionsEMMenuButton->GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsEMMenuButton->SetEnabled(tree->GetEnabled());
      sprintf(buffer, "StoppingConditionsEMCallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionIterations);
      menu->AddRadioButton("Iterations", this, buffer);
      sprintf(buffer, "StoppingConditionsEMCallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure);
      menu->AddRadioButton("Label Map", this, buffer);
      sprintf(buffer, "StoppingConditionsEMCallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure);
      menu->AddRadioButton("Weights", this, buffer);
      vtksys_stl::string value;
      switch (mrmlManager->GetTreeNodeStoppingConditionEMType(sel_vol_id))
        {
        case vtkEMSegmentMRMLManager::StoppingConditionIterations:
          value = "Iterations";
          break;
        case vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure:
          value = "Label Map";
          break;
        case vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure:
          value = "Weights";
          break;
        }
      this->StoppingConditionsEMMenuButton->GetWidget()->SetValue(
        value.c_str());
      }
    else
      {
      this->StoppingConditionsEMMenuButton->GetWidget()->SetValue("");
      this->StoppingConditionsEMMenuButton->SetEnabled(0);
      }
    }

  // Update the EM iterations entry

  if (this->StoppingConditionsEMIterationsEntry)
    {
    vtkKWEntry *entry =
      this->StoppingConditionsEMIterationsEntry->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsEMIterationsEntry->SetEnabled(enabled);
      sprintf(buffer, "StoppingConditionsEMIterationsCallback %d",
          static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsInt(
        mrmlManager->GetTreeNodeStoppingConditionEMIterations(sel_vol_id));
      }
    else
      {
      this->StoppingConditionsEMIterationsEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      }
    if (has_valid_selection && !sel_is_leaf_node &&
        mrmlManager->GetTreeNodeStoppingConditionEMType(sel_vol_id) ==
        vtkEMSegmentMRMLManager::StoppingConditionIterations)
      {
      this->Script("grid %s",
          this->StoppingConditionsEMIterationsEntry->GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",
          this->StoppingConditionsEMIterationsEntry->GetWidgetName());
      }
    }

  // Update the EM value entry

  if (this->StoppingConditionsEMValueEntry)
    {
    vtkKWEntry *entry = this->StoppingConditionsEMValueEntry->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsEMValueEntry->SetEnabled(enabled);
      sprintf(buffer, "StoppingConditionsEMValueCallback %d",
          static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsDouble(
        mrmlManager->GetTreeNodeStoppingConditionEMValue(sel_vol_id));
      }
    else
      {
      this->StoppingConditionsEMValueEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      }
    if (has_valid_selection && !sel_is_leaf_node &&
        (mrmlManager->GetTreeNodeStoppingConditionEMType(sel_vol_id) ==
         vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure ||
         mrmlManager->GetTreeNodeStoppingConditionEMType(sel_vol_id) ==
         vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure))
      {
      this->Script("grid %s",this->StoppingConditionsEMValueEntry->
          GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",this->StoppingConditionsEMValueEntry->
          GetWidgetName());
      }
    }

  // Update the MFA menu button

  if (this->StoppingConditionsMFAMenuButton)
    {
    vtkKWMenu *menu = this->StoppingConditionsMFAMenuButton->GetWidget()->
      GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsMFAMenuButton->SetEnabled(tree->
          GetEnabled());
      sprintf(buffer, "StoppingConditionsMFACallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionIterations);
      menu->AddRadioButton("Iterations", this, buffer);
      sprintf(buffer, "StoppingConditionsMFACallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure);
      menu->AddRadioButton("Label Map", this, buffer);
      sprintf(buffer, "StoppingConditionsMFACallback %d %d",
          static_cast<int>(sel_vol_id),
          vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure);
      menu->AddRadioButton("Weights", this, buffer);
      vtksys_stl::string value;
      switch (mrmlManager->GetTreeNodeStoppingConditionMFAType(sel_vol_id))
        {
        case vtkEMSegmentMRMLManager::StoppingConditionIterations:
          value = "Iterations";
          break;
        case vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure:
          value = "Label Map";
          break;
        case vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure:
          value = "Weights";
          break;
        }
      this->StoppingConditionsMFAMenuButton->GetWidget()->SetValue(
        value.c_str());
      }
    else
      {
      this->StoppingConditionsMFAMenuButton->GetWidget()->SetValue("");
      this->StoppingConditionsMFAMenuButton->SetEnabled(0);
      }
    }

  // Update the MFA iterations entry

  if (this->StoppingConditionsMFAIterationsEntry)
    {
    vtkKWEntry *entry =
      this->StoppingConditionsMFAIterationsEntry->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsMFAIterationsEntry->SetEnabled(enabled);
      sprintf(buffer, "StoppingConditionsMFAIterationsCallback %d",
              static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsInt(
          mrmlManager->GetTreeNodeStoppingConditionMFAIterations(
            sel_vol_id));
      }
    else
      {
      this->StoppingConditionsMFAIterationsEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      }
    if (has_valid_selection && !sel_is_leaf_node &&
        mrmlManager->GetTreeNodeStoppingConditionMFAType(sel_vol_id) ==
        vtkEMSegmentMRMLManager::StoppingConditionIterations)
      {
      this->Script("grid %s",
          this->StoppingConditionsMFAIterationsEntry->GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",
          this->StoppingConditionsMFAIterationsEntry->GetWidgetName());
      }
    }

  // Update the MFA value entry

  if (this->StoppingConditionsMFAValueEntry)
    {
    vtkKWEntry *entry = this->StoppingConditionsMFAValueEntry->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsMFAValueEntry->SetEnabled(enabled);
      sprintf(buffer, "StoppingConditionsMFAValueCallback %d",
              static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsDouble(mrmlManager->
          GetTreeNodeStoppingConditionMFAValue(sel_vol_id));
      }
    else
      {
      this->StoppingConditionsMFAValueEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      }
    if (has_valid_selection && !sel_is_leaf_node &&
        (mrmlManager->GetTreeNodeStoppingConditionMFAType(sel_vol_id) ==
         vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure ||
         mrmlManager->GetTreeNodeStoppingConditionMFAType(sel_vol_id) ==
         vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure))
      {
      this->Script("grid %s",this->StoppingConditionsMFAValueEntry->
          GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",this->StoppingConditionsMFAValueEntry->
          GetWidgetName());
      }
    }

  // Update the Bias Calculation Max Iterations entry

  if (this->StoppingConditionsBiasIterationsEntry)
    {
    vtkKWEntry *entry =
      this->StoppingConditionsBiasIterationsEntry->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer,"StoppingConditionsBiasIterationsCallback %d",
          static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsInt(mrmlManager->
          GetTreeNodeBiasCalculationMaxIterations(sel_vol_id));
      this->Script("grid %s",this->StoppingConditionsBiasIterationsEntry->
          GetWidgetName());
      }
    else
      {
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      this->Script("grid remove %s",this->
          StoppingConditionsBiasIterationsEntry->GetWidgetName());
      }
    }

  // Update the print weight

  if (this->NodeParametersPrintWeightCheckButton)
    {
    vtkKWCheckButton *cb = this->NodeParametersPrintWeightCheckButton->
      GetWidget();
    if (has_valid_selection)
      {
      this->NodeParametersPrintWeightCheckButton->SetEnabled(enabled);
      sprintf(buffer, "NodeParametersPrintWeightCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintWeight(
            sel_vol_id));
      }
    else
      {
      this->NodeParametersPrintWeightCheckButton->SetEnabled(0);
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      }
    }

  // Update the print quality

  if (this->NodeParametersPrintQualityCheckButton)
    {
    vtkKWCheckButton *cb = this->NodeParametersPrintQualityCheckButton->
      GetWidget();
    if (has_valid_selection && sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintQualityCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintQuality(
            sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
          this->NodeParametersPrintQualityCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script( "pack forget %s",this->
          NodeParametersPrintQualityCheckButton->GetWidgetName());
      }
    }

  // Update the print frequency

  if (this->NodeParametersPrintFrequencyScale)
    {
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(
        buffer, "NodeParametersPrintFrequencyChangedCallback %d",
        static_cast<int>(sel_vol_id));
      this->NodeParametersPrintFrequencyScale->SetEndCommand(this, buffer);
      this->NodeParametersPrintFrequencyScale->SetEntryCommand(this,
          buffer);
      this->NodeParametersPrintFrequencyScale->
        SetValue(mrmlManager->GetTreeNodePrintFrequency(sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
          this->NodeParametersPrintFrequencyScale->GetWidgetName());
      }
    else
      {
      this->NodeParametersPrintFrequencyScale->SetEndCommand(NULL, NULL);
      this->NodeParametersPrintFrequencyScale->SetEntryCommand(NULL, NULL);
      this->Script("pack forget %s",
          this->NodeParametersPrintFrequencyScale->GetWidgetName());
      }
    }

  // Update the print bias

  if (this->NodeParametersPrintBiasCheckButton)
    {
    vtkKWCheckButton *cb =
      this->NodeParametersPrintBiasCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintBiasCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintBias(sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
          this->NodeParametersPrintBiasCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script("pack forget %s",this->
          NodeParametersPrintBiasCheckButton->GetWidgetName());
      }
    }

  // Update the print label map

  if (this->NodeParametersPrintLabelMapCheckButton)
    {
    vtkKWCheckButton *cb =
      this->NodeParametersPrintLabelMapCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintLabelMapCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintLabelMap(
            sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
          this->NodeParametersPrintLabelMapCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script( "pack forget %s",this->
          NodeParametersPrintLabelMapCheckButton->GetWidgetName());
      }
    }

  // Update the print EM label map

  if (this->NodeParametersPrintEMLabelMapConvergenceCheckButton)
    {
    vtkKWCheckButton *cb =
      this->NodeParametersPrintEMLabelMapConvergenceCheckButton->
      GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintEMLabelMapCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintEMLabelMapConvergence(
            sel_vol_id));
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      }
    }

  // Update the print EM weights

  if (this->NodeParametersPrintEMWeightsConvergenceCheckButton)
    {
    vtkKWCheckButton *cb = this->
      NodeParametersPrintEMLabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintEMWeightsCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintEMWeightsConvergence(
            sel_vol_id));
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      }
    }

  // Update the print MFA label map

  if (this->NodeParametersPrintMFALabelMapConvergenceCheckButton)
    {
    vtkKWCheckButton *cb = this->
      NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintMFALabelMapCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->
          GetTreeNodePrintMFALabelMapConvergence(sel_vol_id));
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      }
    }

  // Update the print MFA label map

  if (this->NodeParametersPrintMFAWeightsConvergenceCheckButton)
    {
    vtkKWCheckButton *cb = this->
      NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintMFAWeightsCallback %d",
          static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->
          GetTreeNodePrintMFAWeightsConvergence(sel_vol_id));
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      }
    }

  // Update the print convergence frame
  if (this->PrintConvergenceFrame)
    {
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->Script(
        "pack %s -side left -anchor nw -fill x -expand y -padx 5 -pady 2",
        this->PrintConvergenceFrame->GetWidgetName());
      }
    else
      {
      this->Script("pack forget %s",
        this->PrintConvergenceFrame->GetWidgetName());
      }
    }

  // Update the class interaction matrices

  if (this->NodeParametersInteractionMatricesFrame)
    {
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->Script(
        "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->NodeParametersInteractionMatricesFrame->GetWidgetName());
      }
    else
      {
      this->Script("pack forget %s",
        this->NodeParametersInteractionMatricesFrame->GetWidgetName());
      }
    }

  // Update the inhomogeneity matrices

  if (this->NodeParametersInhomogeneityFrame)
    {
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->Script(
        "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
        this->NodeParametersInhomogeneityFrame->GetWidgetName());
      }
    else
      {
      this->Script("pack forget %s",
        this->NodeParametersInhomogeneityFrame->GetWidgetName());
      }
    }

  // Update miscellaeneous Exclude from incomplete EStep parameters

  if(this->NodeParametersExcludeIncompleteEStepCheckButton)
    {
    vtkKWCheckButton *cb =
      this->NodeParametersExcludeIncompleteEStepCheckButton->GetWidget();
    if (has_valid_selection)
      {
      sprintf(buffer, "ExcludeIncompleteEStepCallback %d",
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodeExcludeFromIncompleteEStep(sel_vol_id));
      this->NodeParametersExcludeIncompleteEStepCheckButton->SetEnabled(
        enabled);
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->NodeParametersExcludeIncompleteEStepCheckButton->SetEnabled(0);
      }
    }

  // Update miscellaeneous Generate Background probability parameters

  if(this->NodeParametersGenerateBackgroundProbabilityCheckButton)
    {
    vtkKWCheckButton *cb =
     this->NodeParametersGenerateBackgroundProbabilityCheckButton->
     GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "GenerateBackgroundProbabilityCallback %d",
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodeGenerateBackgroundProbability(sel_vol_id));
      this->Script(
        "pack %s -side top -anchor nw -padx 2 -pady 2",
        this->NodeParametersGenerateBackgroundProbabilityCheckButton
        ->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script("pack forget %s",
        this->NodeParametersGenerateBackgroundProbabilityCheckButton
        ->GetWidgetName());
      }
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  NodeParametersGlobalPriorChangedCallback(
    vtkIdType sel_vol_id, double value)
{
  // The class probability has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeClassProbability(sel_vol_id, value);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  NodeParametersSpatialPriorWeightChangedCallback(
    vtkIdType sel_vol_id, double value)
{
  // The spatial prior weight has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeSpatialPriorWeight(sel_vol_id, value);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  NodeParametersInputChannelWeightChangedCallback(
    vtkIdType sel_vol_id, int row, int, const char *value)
{
  // The input channel weight has changed because of user interaction

  double w = atof(value);
  if (w >= 0 && w <= 1.0)
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->
      GetMRMLManager();
    if (mrmlManager)
      {
      mrmlManager->SetTreeNodeInputChannelWeight(sel_vol_id, row,
          atof(value));
      }
    }
  else
    {
    this->DisplaySelectedNodeParametersCallback();
    vtkKWMultiColumnList *list =
      this->NodeParametersInputChannelWeightsList->GetWidget()->
      GetWidget();
    list->SeeRow(row);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersAlphaChangedCallback(
  vtkIdType sel_vol_id, double value)
{
  // The alpha has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeAlpha(sel_vol_id, value);
    }
}

//--------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMCallback(
  vtkIdType sel_vol_id, int value)
{
  // The EM stopping condition has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && value != mrmlManager->
      GetTreeNodeStoppingConditionEMType(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionEMType(sel_vol_id, value);
    this->DisplaySelectedNodeParametersCallback();
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMIterationsCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The EM iterations has changed because of user interaction

  int v = (int)abs(atoi(value));
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && v != mrmlManager->
      GetTreeNodeStoppingConditionEMIterations(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionEMIterations(sel_vol_id, v);
    this->DisplaySelectedNodeParametersCallback(); // in case the value < 0
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMValueCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The EM value has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeStoppingConditionEMValue(sel_vol_id,
        atof(value));
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsMFACallback(
  vtkIdType sel_vol_id, int value)
{
  // The MFA stopping condition has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && value != mrmlManager->
      GetTreeNodeStoppingConditionMFAType(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAType(sel_vol_id, value);
    this->DisplaySelectedNodeParametersCallback();
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  StoppingConditionsMFAIterationsCallback(
    vtkIdType sel_vol_id, const char *value)
{
  // The MFA iterations has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int v = (int)abs(atoi(value));
  if (v != mrmlManager->GetTreeNodeStoppingConditionMFAIterations(
        sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAIterations(sel_vol_id, v);
    this->DisplaySelectedNodeParametersCallback(); // in case the value < 0
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsMFAValueCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The MFA value has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAValue(sel_vol_id,
        atof(value));
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  StoppingConditionsBiasIterationsCallback(
    vtkIdType sel_vol_id, const char *value)
{
  // The Bias calculation max iterations value has changed because of
  // user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeBiasCalculationMaxIterations(
      sel_vol_id, atoi(value));
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintWeightCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print weight has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintWeight(sel_vol_id, value);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  NodeParametersPrintQualityCallback(vtkIdType sel_vol_id, int value)
{
  // The print quality has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintQuality(sel_vol_id, value);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  NodeParametersPrintFrequencyChangedCallback(
    vtkIdType sel_vol_id, int value)
{
  // The print frequency has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintFrequency(sel_vol_id, value);
    }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintBiasCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print bias has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintBias(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintLabelMapCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print label map has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintLabelMap(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintEMLabelMapCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print EM label map convergence has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintEMLabelMapConvergence(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintEMWeightsCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print EM weight convergence has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintEMWeightsConvergence(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintMFALabelMapCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print MFA label map convergence has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintMFALabelMapConvergence(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintMFAWeightsCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print MFA weight convergence has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintMFAWeightsConvergence(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ExcludeIncompleteEStepCallback(
  vtkIdType sel_vol_id, int state)
{
  // The exclude from incomplete EStep checkbox has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeExcludeFromIncompleteEStep(sel_vol_id, state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GenerateBackgroundProbabilityCallback(
  vtkIdType sel_vol_id, int state)
{
  // The generate background probability checkbox has changed
  // because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeGenerateBackgroundProbability(sel_vol_id, state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::Validate()
{
  vtkKWWizardWorkflow *wizard_workflow = this->GetGUI()->GetWizardWidget()->
    GetWizardWorkflow();
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  // make sure that the probability for each set of sibling nodes sums
  // to unity

  if (mrmlManager->GetTreeRootNode() != NULL)
    {
    vtkIdType firstBadTreeID = mrmlManager->
      GetTreeNodeFirstIDWithChildProbabilityError();
    if (firstBadTreeID >= 0)
      {
      std::stringstream ss;
      ss << "Child probabilities must sum to one for node "
         << mrmlManager->GetTreeNodeName(firstBadTreeID)
         << "; right now they sum to "
         << mrmlManager->GetTreeNodeChildrenSumClassProbability(
             firstBadTreeID)
         << ".  Please fix before continuing---"
         << "you should edit the \"Global Prior\" fields for the"
         << " children nodes of "
         << mrmlManager->GetTreeNodeName(firstBadTreeID) << ".";

      std::string parentNodeName = mrmlManager->GetTreeNodeName(
          firstBadTreeID);
      std::string errorMessage   = parentNodeName  +
        ": Child probabilities must sum to one!  " +
        "Please fix before continuing.";
      vtkKWMessageDialog::PopupMessage(this->GetApplication(),
          NULL,
          "Node Parameters Error",
          ss.str().c_str(),
          vtkKWMessageDialog::ErrorIcon |
          vtkKWMessageDialog::InvokeAtPointer);

      // there was an error; stay on this step
      wizard_workflow->
        PushInput(vtkKWWizardStep::GetValidationFailedInput());
      wizard_workflow->ProcessInputs();
      }
    }
  this->Superclass::Validate();
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
RightClickOnInputChannelWeightsListCallback(int row, int col, int x, int y)
{
  vtkKWMultiColumnList *list = this->NodeParametersInputChannelWeightsList->
    GetWidget()->GetWidget();
  list->EditCell(row, col);
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddPointMovingGUIEvents()
{
  this->IntensityDistributionHistogramHistogramVisualization->AddObserver(
      vtkKWParameterValueFunctionEditor::PointChangingEvent, this->GetGUI()->
      GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemovePointMovingGUIEvents()
{
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessPointMovingGUIEvents(
  vtkObject *caller, unsigned long event, void *callData)
{
  if(event == vtkKWParameterValueFunctionEditor::PointChangingEvent)
  {
    int id = 2;
    double nodeValue[6];

    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id,
        nodeValue);

    this->Test();
  }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddPointAddGUIEvents()
{
  this->IntensityDistributionHistogramHistogramVisualization->AddObserver(
      vtkKWParameterValueFunctionEditor::PointAddedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemovePointAddGUIEvents()
{
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessPointAddGUIEvents(
  vtkObject *caller, unsigned long event, void *callData)
{
  if(event == vtkKWParameterValueFunctionEditor::PointAddedEvent)
  {
    this->Size = this->IntensityDistributionHistogramHistogramVisualization->
      GetFunctionSize();
  }
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::Test()
{
  double midmin=0.0, midmax=0.0;
  double nodeValueNext[6], nodeValuePrev[6], nodeValue[6];
  double *position = new double[this->NumberOfLeaves]; //6

  for(int i=0; i < this->NumberOfLeaves; i++)
  {
    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i,
        nodeValue);
    position[i] = nodeValue[0];
  }

  for(int i=1; i < this->NumberOfLeaves-1; i++)
  {
    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(
        i-1, nodeValuePrev);
    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i,
        nodeValue);
    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i+1,
     nodeValueNext);

    midmin = nodeValue[0] - (position[i] - position[i-1]) *
      (1-nodeValuePrev[4]);
    midmax = nodeValueNext[0] - (position[i+1] - position[i]) *
      (1-nodeValue[4]);

    this->ClassSize[i*2]   = midmin;
    this->ClassSize[i*2+1] = midmax;
  }

  if( this->NumberOfLeaves > 2 )
  {
    this->ClassSize[0] = position[0];
    this->ClassSize[1] = this->ClassSize[2];

    this->ClassSize[(this->NumberOfLeaves)*2 - 2] = this->ClassSize[
      (this->NumberOfLeaves)*2-3];
    this->ClassSize[(this->NumberOfLeaves)*2 - 1] = position[
      (this->NumberOfLeaves)-1];
  }
  else
  {
    midmin = 0;

    this->ClassSize[0] = position[0];
    this->ClassSize[3] = position[1];

    this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(0,
        nodeValue);
    midmin = position[0] + (position[1] - position[0])*(nodeValue[4]);

    this->ClassSize[1] = midmin;
    this->ClassSize[2] = midmin;

    std::cout<<"size: "<< this->ClassSize[0] <<" to: "<< this->ClassSize[1]
      << std::endl;
    std::cout<<"size: "<< this->ClassSize[2] <<" to: "<< this->ClassSize[3]
      << std::endl;
  }

  delete position;
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetNumberOfLeaf(
  const char *parent, vtkIdType vol_id)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int numChildren = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);

  if (numChildren == 0)
  {
    this->NumberOfLeaves = this->NumberOfLeaves +1;
    this->LeafId[this->NumberOfLeaves-1] = static_cast<int>(vol_id);
    std::cout <<"ROOT ID: "<< mrmlManager->GetTreeRootNodeID()<< std::endl;
  }

  for (int i=0; i < numChildren; i++)
    {
    this->GetNumberOfLeaf(NULL,mrmlManager->GetTreeNodeChildNodeID(vol_id,i));
    }
}
//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetParentPercent(
  int i, vtkIdType vol_id)
{
  this->Depth = this->Depth +1 ;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  vtkIdType parent_ID = mrmlManager->GetTreeNodeParentNodeID(vol_id);

  if(parent_ID != mrmlManager->GetTreeRootNodeID()){

  this->ClassPercentOrder[this->Depth][i] = parent_ID;
  this->ClassPercentOrderCP[this->Depth][i] = parent_ID;
  this->GetParentPercent(i, parent_ID);
  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessTestButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{
  if(event == vtkKWPushButton::InvokedEvent)
  {
    for(int i = 0;i < this->NumberOfLeaves;i++)
    {
      this->Depth = 0;
      this->ClassPercentOrder[0][i] = this->LeafId[i];
      this->ClassPercentOrderCP[0][i] = this->LeafId[i];
      this->GetParentPercent(i,this->LeafId[i]);
    }

    int control = 1;
    int position = 0;
    int positionF ;

    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

    for(int i=0; i < this->NumberOfLeaves; i++)
    {
      for(int j=0; j < 200; j++)
      {
        for(int k=0; k < this->NumberOfLeaves; k++)
        {
          for(int l=0; l < 200; l++)
          {
            if (this->ClassPercentOrder[j][i] ==
                this->ClassPercentOrderCP[l][k] &&
                this->ClassPercentOrder[j][i] != 0)
            {
              this->ClassPercentOrderCP[l][k] = 0;

              for(int m=0; m < 200; m++)
              {
                if(this->correspondanceArray[0][m] ==
                    this->ClassPercentOrder[j][i])
                {
                  positionF = m;
                  control = 0;
                }
              }

              if (control != 0)
              {
                positionF = position;
                position++;
              }

              this->correspondanceArray[0][positionF] =
                this->ClassPercentOrder[j][i];

              for(int z=0; z < this->NumberOfLeaves; z++)
              {
                if (strcmp(this->ClassAndNodeList->GetCellText(z,0),
                      mrmlManager->GetTreeNodeName(this->
                        ClassPercentOrder[0][k])) == 0)
                {
                  this->ClassWeight[positionF] = this->ClassWeight[positionF]
                    + this->GetWeight(z); //remplace 1 by
                }
              }

              control = 1;
            }
          }
        }
      }
    }

    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
    {
      this->GetPercent(3,root_id);
    }
  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddTestButtonGUIEvents()
{
  this->TestButton->AddObserver(vtkKWPushButton::InvokedEvent,this->GetGUI()->
      GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemoveTestButtonGUIEvents()
{
}

//-------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetPercent(int j, vtkIdType vol_id)
{
  this->Depth = this->Depth + 1;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int numChildren = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  double weight = 0.0;

  if(numChildren > 0)
  {
    for (int i=0; i < numChildren; i++)
    {
      for(int m=0; m < 200; m++)
      {
        if(this->correspondanceArray[0][m] == mrmlManager->
            GetTreeNodeChildNodeID(vol_id, i))
        {
          weight += this->ClassWeight[m];
        }
      }
    }

    if (weight > 0.0)
    {
      double factor = 1.0 / weight;

      for (int i = 0; i < numChildren; i++)
      {
        for (int m = 0; m < 200; m++)
        {
          if (this->correspondanceArray[0][m] == mrmlManager->
              GetTreeNodeChildNodeID(vol_id, i))
          {
            this->ClassWeight[m] = factor * this->ClassWeight[m];
          }
        }
      }
    }

    for (int i = 0; i < numChildren; i++)
    {
      this->GetPercent(3,mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
    }

    for (int i = 0; i < 200; i++)
    {
      if (this->correspondanceArray[0][i] != 0)
      {
        mrmlManager->SetTreeNodeClassProbability(
            this->correspondanceArray[0][i], this->ClassWeight[i]);
      }
    }
  }
}

//-------------------------------------------------------------------
double vtkEMSegmentNodeParametersStep::GetWeight(int z)
{
  double sum = 0.0;

  int start = (int) (this->ClassSize[z*2] + 0.5) + 1;
  int stop  = (int) (this->ClassSize[z*2 + 1] + 0.5);

  for( int i=start; i < stop; i++)
    {
      sum += this->IntensityDistributionHistogramHistogram->
        GetOccurenceAtValue(i);
    }

  return sum;
}

