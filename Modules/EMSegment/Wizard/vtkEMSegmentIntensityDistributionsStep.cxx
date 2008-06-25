#include "vtkEMSegmentIntensityDistributionsStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnListWithScrollbarsWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWNotebook.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkEMSegmentAnatomicalStructureStep.h"

#include "vtkSlicerInteractorStyle.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityDistributionsStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityDistributionsStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::vtkEMSegmentIntensityDistributionsStep()
{
  this->SetName("6/9. Specify Intensity Distributions");
  this->SetDescription(
    "Define intensity distribution for each anatomical structure.");

  this->IntensityDistributionNotebook                = NULL;
  this->IntensityDistributionSpecificationMenuButton = NULL;
  this->IntensityDistributionMeanMatrix              = NULL;
  this->IntensityDistributionCovarianceMatrix        = NULL;
  this->IntensityDistributionManualSamplingList      = NULL;
  this->ContextMenu  = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::~vtkEMSegmentIntensityDistributionsStep()
{
  if (this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook->Delete();
    this->IntensityDistributionNotebook = NULL;
    }

  if (this->IntensityDistributionSpecificationMenuButton)
    {
    this->IntensityDistributionSpecificationMenuButton->Delete();
    this->IntensityDistributionSpecificationMenuButton = NULL;
    }

  if (this->IntensityDistributionMeanMatrix)
    {
    this->IntensityDistributionMeanMatrix->Delete();
    this->IntensityDistributionMeanMatrix = NULL;
    }

  if (this->IntensityDistributionCovarianceMatrix)
    {
    this->IntensityDistributionCovarianceMatrix->Delete();
    this->IntensityDistributionCovarianceMatrix = NULL;
    }

  if (this->IntensityDistributionManualSamplingList)
    {
    this->IntensityDistributionManualSamplingList->Delete();
    this->IntensityDistributionManualSamplingList = NULL;
    }

  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  anat_step->ShowAnatomicalStructureTree();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkIdType vol_id = mrmlManager->GetTreeRootNodeID();
  const char *root_node = 
    anat_step->GetAnatomicalStructureTree()->GetWidget()->FindNodeWithUserDataAsInt(NULL, vol_id);
  if (root_node && *root_node)
    {
    anat_step->SetAnatomicalTreeParentNodeSelectableState(root_node, 0);
    }

  this->AddManualIntensitySamplingGUIObservers();
  
  // Override the tree callbacks for that specific step

  anat_step->GetAnatomicalStructureTree()->GetWidget()->SetSelectionChangedCommand(
      this, "DisplaySelectedNodeIntensityDistributionsCallback");

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Create the notebook

  if (!this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook = vtkKWNotebook::New();
    }
  if (!this->IntensityDistributionNotebook->IsCreated())
    {
    this->IntensityDistributionNotebook->SetParent(parent);
    this->IntensityDistributionNotebook->Create();
    this->IntensityDistributionNotebook->AddPage(
      "Intensity Distribution");
    this->IntensityDistributionNotebook->AddPage(
      "Manual Sampling");
    }
  vtkKWFrame *intensity_page = 
    this->IntensityDistributionNotebook->GetFrame("Intensity Distribution");
  vtkKWFrame *manual_sampling_page = 
    this->IntensityDistributionNotebook->GetFrame("Manual Sampling");

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
    this->IntensityDistributionNotebook->GetWidgetName());

  // Create the distribution specification menu button

  if (!this->IntensityDistributionSpecificationMenuButton)
    {
    this->IntensityDistributionSpecificationMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->IntensityDistributionSpecificationMenuButton->IsCreated())
    {
    this->IntensityDistributionSpecificationMenuButton->SetParent(
      intensity_page);
    this->IntensityDistributionSpecificationMenuButton->Create();
    this->IntensityDistributionSpecificationMenuButton->
      SetLabelText("Specification:");
    this->IntensityDistributionSpecificationMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->SetBalloonHelpString(
      "Select intensity distribution specification type.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->IntensityDistributionSpecificationMenuButton->GetWidgetName());

  // Create the distribution mean vector/matrix

  if (!this->IntensityDistributionMeanMatrix)
    {
    this->IntensityDistributionMeanMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->IntensityDistributionMeanMatrix->IsCreated())
    {
    this->IntensityDistributionMeanMatrix->SetParent(intensity_page);
    this->IntensityDistributionMeanMatrix->Create();
    this->IntensityDistributionMeanMatrix->SetLabelText("Log Mean:");
    this->IntensityDistributionMeanMatrix->ExpandWidgetOff();
    this->IntensityDistributionMeanMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionMeanMatrix->SetBalloonHelpString(
      "Set the intensity distribution mean.");
    
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionMeanMatrix->GetWidget();
    matrix->SetNumberOfColumns(0);
    matrix->SetNumberOfRows(0);
    matrix->SetElementWidth(6);
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  this->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2",
               this->IntensityDistributionMeanMatrix->GetWidgetName());

  // Create the distribution covariance vector/matrix

  if (!this->IntensityDistributionCovarianceMatrix)
    {
    this->IntensityDistributionCovarianceMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->IntensityDistributionCovarianceMatrix->IsCreated())
    {
    this->IntensityDistributionCovarianceMatrix->SetParent(intensity_page);
    this->IntensityDistributionCovarianceMatrix->Create();
    this->IntensityDistributionCovarianceMatrix->SetLabelText(
      "Log Covariance:");
    this->IntensityDistributionCovarianceMatrix->ExpandWidgetOff();
    this->IntensityDistributionCovarianceMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionCovarianceMatrix->SetBalloonHelpString(
      "Set the intensity distribution covariance.");
    
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionCovarianceMatrix->GetWidget();
    matrix->SetNumberOfColumns(0);
    matrix->SetNumberOfRows(0);
    matrix->SetElementWidth(6);
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  this->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2",
               this->IntensityDistributionCovarianceMatrix->GetWidgetName());
  
  // Create the manual sampling frame

  if (!this->IntensityDistributionManualSamplingList)
    {
    this->IntensityDistributionManualSamplingList = 
      vtkKWMultiColumnListWithScrollbarsWithLabel::New();
    }
  if (!this->IntensityDistributionManualSamplingList->IsCreated())
    {
    this->IntensityDistributionManualSamplingList->SetParent(
      manual_sampling_page);
    this->IntensityDistributionManualSamplingList->Create();
    this->IntensityDistributionManualSamplingList->SetLabelPositionToTop();
    this->IntensityDistributionManualSamplingList->SetLabelText(
      "Control+Click in a slice window to pick a sample.");
    this->IntensityDistributionManualSamplingList->GetWidget()->
      HorizontalScrollbarVisibilityOff();

    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    list->SetRightClickCommand(
      this, "PopupManualIntensitySampleContextMenuCallback");
    list->SetHeight(4);
    list->MovableColumnsOff();
    list->SetSelectionModeToSingle();
    list->ResizableColumnsOff();
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2", 
    this->IntensityDistributionManualSamplingList->GetWidgetName());

  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeIntensityDistributionsCallback();
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveManualIntensitySamplingGUIObservers();
}

//----------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DisplaySelectedNodeIntensityDistributionsCallback()
{
  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  if (!anat_step)
    {
    return;
    }
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id = 0;
  int manually_sample_mode = 0;
  int has_valid_selection = tree->HasSelection();
  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    has_valid_selection = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    manually_sample_mode = 
      mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) ==
      vtkEMSegmentMRMLManager::
      DistributionSpecificationManuallySample;
    }

  int enabled = tree->GetEnabled();
  int row, col;
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];

  // Update the distribution specification menu button

  if (this->IntensityDistributionSpecificationMenuButton)
    {
    vtkKWMenu *menu = this->IntensityDistributionSpecificationMenuButton->
      GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection)
      {
      vtksys_stl::string value;
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(enabled);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id), vtkEMSegmentMRMLManager::
        DistributionSpecificationManual);
      menu->AddRadioButton("Manual", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id),vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample);
      menu->AddRadioButton("Manual Sampling", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d", 
        static_cast<int>(sel_vol_id), vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample);
      menu->AddRadioButton("Auto Sampling", this, buffer);

      // temporarily disable auto sampling because it is not currently
      // implemented
      menu->SetItemStateToDisabled("Auto Sampling");

      switch (mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id))
        {
        case vtkEMSegmentMRMLManager::DistributionSpecificationManual:
          value = "Manual";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample:
          value = "Manual Sampling";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample:
          value = "Auto Sampling";
          break;
        }
      this->IntensityDistributionSpecificationMenuButton->GetWidget()->
        SetValue(value.c_str());
      }
    else
      {
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(0);
      this->IntensityDistributionSpecificationMenuButton->GetWidget()->
        SetValue("");
      }
    }

  // Update the distribution mean vector/matrix

  if (this->IntensityDistributionMeanMatrix)
    {
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionMeanMatrix->GetWidget();
    if (has_valid_selection)
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(1);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer, "IntensityDistributionMeanChangedCallback %d", 
        static_cast<int>(sel_vol_id));
      matrix->SetElementChangedCommand(this, buffer);

      for(col = 0; col < nb_of_target_volumes; col++)
        {
        matrix->SetElementValueAsDouble(
          0, col, 
          mrmlManager->GetTreeNodeDistributionLogMean(sel_vol_id, col));
        }
      }
    else
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(0);
      matrix->SetNumberOfColumns(0);
      matrix->SetElementChangedCommand(NULL, NULL);
      }
    }

  // Update the distribution covariance vector/matrix

  if (this->IntensityDistributionCovarianceMatrix)
    {
    vtkKWMatrixWidget *matrix = 
      this->IntensityDistributionCovarianceMatrix->GetWidget();
    if (has_valid_selection)
      {
      this->IntensityDistributionCovarianceMatrix->SetEnabled(
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(nb_of_target_volumes);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer,"IntensityDistributionCovarianceChangedCallback %d",
        static_cast<int>(sel_vol_id));
      matrix->SetElementChangedCommand(this, buffer);

      for (row = 0; row < nb_of_target_volumes; row++)
        {
        for (col = 0; col < nb_of_target_volumes; col++)
          {
          matrix->SetElementValueAsDouble(
            row, col, 
            mrmlManager->GetTreeNodeDistributionLogCovariance(
              sel_vol_id, row, col));
          }
        }
      }
    else
      {
      this->IntensityDistributionCovarianceMatrix->SetEnabled(0);
      matrix->SetNumberOfColumns(0);
      matrix->SetElementChangedCommand(NULL, NULL);
      }
    }

  // Update the manual sampling list

  if (this->IntensityDistributionNotebook)
    {
    this->IntensityDistributionNotebook->SetPageEnabled(
      "Manual Sampling", manually_sample_mode);
    }

  if (this->IntensityDistributionManualSamplingList)
    {
    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    list->DeleteAllRows();
    if (has_valid_selection && manually_sample_mode)
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(enabled);
      int nb_cols = list->GetNumberOfColumns();
      for (;nb_cols < nb_of_target_volumes; nb_cols++)
        {
        int col_id = list->AddColumn("");
        list->SetColumnWidth(col_id, 0);
        list->ColumnStretchableOff(col_id);
        list->SetColumnSortMode(col_id, vtkKWMultiColumnList::SortModeReal);
        }
      for (;nb_cols > nb_of_target_volumes; nb_cols--)
        {
        list->DeleteColumn(nb_cols - 1);
        }
      for (col = 0; col < nb_of_target_volumes; ++col)
        {
        vtkIdType volumeID = 
          mrmlManager->GetTargetSelectedVolumeNthID(col);
        const char* title = mrmlManager->GetVolumeName(volumeID);
        list->SetColumnTitle(col, title);
        }
      double intensity;
      int nb_samples = 
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(sel_vol_id);
      for (row = 0; row < nb_samples; row++)
        {
        list->AddRow();
        for (col = 0; col < nb_of_target_volumes; col++)
          {
          int vol_id = mrmlManager->GetTargetSelectedVolumeNthID(col);
          intensity = mrmlManager->GetTreeNodeDistributionSampleIntensityValue(
            sel_vol_id, row, vol_id);
          list->SetCellTextAsDouble(row, col, intensity);
          }
        }
      }
    else
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::IntensityDistributionSpecificationCallback(
  vtkIdType sel_vol_id, int type)
{
  // The distribution specification has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  if (type != mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id))
    {
    mrmlManager->SetTreeNodeDistributionSpecificationMethod(sel_vol_id, type);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
    }
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::IntensityDistributionMeanChangedCallback(
  vtkIdType sel_vol_id, int row, int col, const char *value)
{
  // The distribution mean vector has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogMean(sel_vol_id, col, atof(value));
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::IntensityDistributionCovarianceChangedCallback(
  vtkIdType sel_vol_id, int row, int col, const char *value)
{
  // The distribution covariance matrix has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogCovariance(sel_vol_id, row,col,atof(value));
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddIntensityDistributionSamplePoint(
  double ras[3])
{
  // Since it is not a callback, make sure we are really allowed to add
  // a sample point now

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  if (!wizard_widget ||
      wizard_widget->GetWizardWorkflow()->GetCurrentStep() != 
      this)
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  if (!anat_step)
    {
    return;
    }
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id;
  if (tree->HasSelection())
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    if (sel_node.size() &&
        mrmlManager->GetTreeNodeIsLeaf(sel_vol_id) &&
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(sel_vol_id) ==
        vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample)
      {
      mrmlManager->AddTreeNodeDistributionSamplePoint(sel_vol_id, ras);
      this->DisplaySelectedNodeIntensityDistributionsCallback();
      int nb_samples = 
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(sel_vol_id);
      vtkKWMultiColumnList *list = 
       this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
      list->SeeRow(nb_samples - 1);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::PopupManualIntensitySampleContextMenuCallback(int row, int, int x, int y)
{
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  if (!tree->HasSelection())
    {
    return;
    }

  vtksys_stl::string sel_node(tree->GetSelection());
  vtkIdType sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());

  vtkKWMultiColumnList *list = 
    this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
  list->SelectSingleRow(row);

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(list);
    this->ContextMenu->Create();
    }

  char buffer[256];
  this->ContextMenu->DeleteAllItems();

  if (row >= 0)
    {
    sprintf(
      buffer, "DeleteManualIntensitySampleCallback %d %d", 
      static_cast<int>(sel_vol_id), row);
    this->ContextMenu->AddCommand("Delete sample", this, buffer);
    }

  if (list->GetNumberOfRows())
    {
    sprintf(buffer, "DeleteAllManualIntensitySampleCallback %d", 
            static_cast<int>(sel_vol_id));
    this->ContextMenu->AddCommand("Delete all samples", this, buffer);
    }

  this->ContextMenu->PopUp(x, y);
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DeleteManualIntensitySampleCallback(
  vtkIdType sel_vol_id, int sample_index)
{
  // A sample has been deleted because of user interaction

  if (sample_index >= 0)
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (!mrmlManager)
      {
      return;
      }
    vtkKWMultiColumnList *list = 
      this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
    if (!list)
      {
      return;
      }
    list->DeleteRow(sample_index);
    mrmlManager->RemoveTreeNodeDistributionSamplePoint(sel_vol_id, sample_index);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
    }
}

//---------------------------------------------------------------------------
void 
vtkEMSegmentIntensityDistributionsStep::DeleteAllManualIntensitySampleCallback(vtkIdType sel_vol_id)
{
  // All samples have been deleted because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWMultiColumnList *list = 
    this->IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
  if (!mrmlManager || !list)
    {
    return;
    }
  list->DeleteAllRows();
  mrmlManager->RemoveAllTreeNodeDistributionSamplePoints(sel_vol_id);
  this->DisplaySelectedNodeIntensityDistributionsCallback();
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddManualIntensitySamplingGUIObservers() 
{
  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveManualIntensitySamplingGUIObservers()
{
  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent, this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessManualIntensitySamplingGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
  vtkSlicerInteractorStyle *s = vtkSlicerInteractorStyle::SafeDownCast(caller);

  if (s && 
      event == vtkCommand::LeftButtonPressEvent &&
      s->GetInteractor()->GetControlKey())
    {
    // Slice GUI 0

    vtkSlicerSliceGUI *sliceGUI0 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red");

    vtkRenderWindowInteractor *rwi0 = sliceGUI0->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    // Slice GUI 1

    vtkSlicerSliceGUI *sliceGUI1 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow");

    vtkRenderWindowInteractor *rwi1 = sliceGUI1->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    // Slice GUI 2

    vtkSlicerSliceGUI *sliceGUI2 = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green");

    vtkRenderWindowInteractor *rwi2 =  sliceGUI2->GetSliceViewer()->
      GetRenderWidget()->GetRenderWindowInteractor();

    vtkSlicerSliceGUI *sliceGUI = NULL;
    vtkRenderWindowInteractor *rwi = NULL;
    if (s == rwi0->GetInteractorStyle())
      {
      sliceGUI = sliceGUI0;
      rwi = rwi0;
      }
    else if (s == rwi1->GetInteractorStyle())
      {
      sliceGUI = sliceGUI1;
      rwi = rwi1;
      }
    else if (s == rwi2->GetInteractorStyle())
      {
      sliceGUI = sliceGUI2;
      rwi = rwi2;
      }

    int point[2];
    rwi->GetLastEventPosition(point);
    double inPt[4] = {point[0], point[1], 0, 1};
    double outPt[4];
    vtkMatrix4x4 *matrix = sliceGUI->GetLogic()->GetSliceNode()->GetXYToRAS();
    matrix->MultiplyPoint(inPt, outPt); 
    double ras[3] = {outPt[0], outPt[1], outPt[2]};

    this->AddIntensityDistributionSamplePoint(ras);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
