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

#include "vtkImageChangeInformation.h"
//TEST RESLICE

#include "vtkImageReader2.h"
#include "vtkMatrix4x4.h"
#include "vtkImageReslice.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkCommand.h"
#include "vtkImageData.h"

//NEW

#include "vtkKWHistogram.h"
#include "vtkKWColorTransferFunctionEditor.h"
//#include "vtkKWColorTransferFunctionEditorDerived.h"
#include "vtkColorTransferFunction.h"
#include "vtkKWParameterValueFunctionInterface.h"
#include "vtkKWParameterValueFunctionEditor.h"

#include "vtkKWPushButton.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkKWEntryWithLabel.h"

#include "vtkKWMultiColumnList.h"

#include "vtkPointData.h"

#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

#include "vtkImageCast.h"

#include "itkUnaryFunctorImageFilter.h"
#include "itkExtractImageFilter.h"

#include "vtkMRMLColorNode.h"
#include "vtkImageReslice.h"
#include "vtkExtractVOI.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMatrix4x4.h"
//#include "class vtkImageData;.h"
//END NEW

#include "vtkEMSegmentAnatomicalStructureStep.h"

#define EMSEG_PRINT_FREQUENCY_MAX 20

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentNodeParametersStep);
vtkCxxRevisionMacro(vtkEMSegmentNodeParametersStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentNodeParametersStep::vtkEMSegmentNodeParametersStep()
{
  this->SetName("7/9. Edit Node-based Parameters");
  this->SetDescription("Specify node-based segmentation parameters.");

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
  this->NodeParametersPrintEMLabelMapConvergenceCheckButton = NULL;
  this->NodeParametersPrintEMWeightsConvergenceCheckButton = NULL;
  this->NodeParametersPrintMFALabelMapConvergenceCheckButton = NULL;
  this->NodeParametersPrintMFAWeightsConvergenceCheckButton = NULL;

  this->NodeParametersInteractionMatricesFrame = NULL;
  this->NodeParametersPCAFrame                 = NULL;
  this->NodeParametersRegistrationFrame        = NULL;
  this->NodeParametersMiscellaeneousFrame      = NULL;
  this->NodeParametersExcludeIncompleteEStepCheckButton = NULL;
  this->NodeParametersGenerateBackgroundProbabilityCheckButton = NULL;
  this->NodeParametersInhomogeneityFrame       = NULL;
  
  //this->UpdateFrame       = NULL;
  //this->UpdatePrior       = NULL;
  
  //NEW
  
  this->IntensityDistributionHistogramButton         = NULL;
  this->IntensityDistributionHistogramHistogram      = NULL;
  this->IntensityDistributionHistogramHistogramVisu  = NULL;
  
  this->NbOfClassesEntryLabel                        = NULL;
  
  this->ClassAndNodeList                             = NULL;
  
  this->TestButton                                   = NULL;
  
  this->PreviewSelector                              = NULL;
  
  //END NEW
}

//----------------------------------------------------------------------------
vtkEMSegmentNodeParametersStep::~vtkEMSegmentNodeParametersStep()
{
//NEW

if (this->PreviewSelector)
    {
    this->PreviewSelector->Delete();
    this->PreviewSelector = NULL;
    }

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

  if (this->NbOfClassesEntryLabel)
    {
    this->NbOfClassesEntryLabel->Delete();
    this->NbOfClassesEntryLabel = NULL;
    }

  if (this->IntensityDistributionHistogramHistogramVisu)
    {
    this->IntensityDistributionHistogramHistogramVisu->Delete();
    this->IntensityDistributionHistogramHistogramVisu = NULL;
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
//END NEW

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

  if(this->NodeParametersPrintMFALabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->Delete();
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton = NULL;
    }
  
  if(this->NodeParametersPrintMFAWeightsConvergenceCheckButton)
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
  
  if(this->NodeParametersExcludeIncompleteEStepCheckButton)
    {
    this->NodeParametersExcludeIncompleteEStepCheckButton->Delete();
    this->NodeParametersExcludeIncompleteEStepCheckButton = NULL;
    }

  if(this->NodeParametersGenerateBackgroundProbabilityCheckButton)
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
  /*  
  if (this->UpdateFrame)
    {
    this->UpdateFrame->Delete();
    this->UpdateFrame = NULL;
    }
    
  if (this->UpdatePrior)
    {
    this->UpdatePrior->Delete();
    this->UpdatePrior = NULL;
    }*/
}

//----------------------------------------------------------------------------
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
    SetSelectionChangedCommand(
      this, "DisplaySelectedNodeParametersCallback");

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
    this->NodeParametersGlobalPriorScale->SetBalloonHelpString("Probability that a voxel belonging to the parent structure will also belong to this structure.  The value must be in the range [0,1].  Global priors for each set of siblings must sum to 1."); 
    }

  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2", 
               this->NodeParametersGlobalPriorScale->GetWidgetName());

  // Create the spatial prior weight scale

  if (!this->NodeParametersSpatialPriorWeightScale)
    {
    this->NodeParametersSpatialPriorWeightScale = vtkKWScaleWithEntry::New();
    }
  if (!this->NodeParametersSpatialPriorWeightScale->IsCreated())
    {
    this->NodeParametersSpatialPriorWeightScale->SetParent(basic_page);
    this->NodeParametersSpatialPriorWeightScale->PopupModeOn();
    this->NodeParametersSpatialPriorWeightScale->Create();
    this->NodeParametersSpatialPriorWeightScale->SetEntryWidth(4);
    this->NodeParametersSpatialPriorWeightScale->SetLabelText(
      "Atlas Weight:");
    this->NodeParametersSpatialPriorWeightScale->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 9);
    this->NodeParametersSpatialPriorWeightScale->SetRange(0.0, 1.0);
    this->NodeParametersSpatialPriorWeightScale->SetResolution(0.01);
    this->NodeParametersSpatialPriorWeightScale->GetEntry()->
      SetCommandTriggerToAnyChange();
    this->NodeParametersSpatialPriorWeightScale->SetBalloonHelpString("Weight of the atlas (spatial prior) in the segmentation decision.  The value must be in the range [0,1], where 0 indicates that the atlas is ignored and 1 indicates the maximum atlas weight."); 
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

    vtkKWMultiColumnList *list = 
      this->NodeParametersInputChannelWeightsList->GetWidget()->GetWidget();
    list->SetHeight(4);
    list->MovableColumnsOff();
    list->SetSelectionModeToSingle();
    list->ResizableColumnsOff();

    int col_id = list->AddColumn("Volume");
    list->SetColumnWidth(col_id, 15);
    list->SetColumnEditable(col_id, 0);
    col_id = list->AddColumn("Weight");
    list->SetColumnEditable(col_id, 1);

    list->SetRightClickCommand
      (this, "RightClickOnInputChannelWeightsListCallback");
    list->SetBalloonHelpString
      ("Weight of each channel in the segmentation decision.  Right-click or double-click to modify weights.  Weights should be in the range [0,1]; 0 indicates that the channel is ignored and 1 indicates the maximum channel weight.  The weights are not dependent on each other or any other weights.");
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
    this->NodeParametersAlphaScale->GetEntry()->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 2 -sticky nw -padx 2 -pady 2", 
               this->NodeParametersAlphaScale->GetWidgetName());
               
  this->Script("grid columnconfigure %s 1 -weight 1", basic_page->GetWidgetName());
      
    ////     NEW
  
  // GET NUMBER OF LEAF
  
  this->nbOfLeaf = 0;
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
   
  if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL, root_id);
      }
    }
    
    // Create the preview volume selector
             
  if (!this->PreviewSelector)
    {
    this->PreviewSelector = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->PreviewSelector->IsCreated())
    {
    this->PreviewSelector->SetNodeClass(
      "vtkMRMLScalarVolumeNode", 
      "LabelMap", "1", 
      "Preview LabelMap");
    this->PreviewSelector->SetNewNodeEnabled(1);
    this->PreviewSelector->SetParent(
      prior_page);
    this->PreviewSelector->Create();
    this->PreviewSelector->
      SetMRMLScene(mrmlManager->GetMRMLScene());

    this->PreviewSelector->SetBorderWidth(2);
    this->PreviewSelector->SetLabelText( "Preview Labelmap: ");
    this->PreviewSelector->SetBalloonHelpString(
      "select an preview labelmap from the current mrml scene.");
    }
  this->PreviewSelector->UpdateMenu();
  if(mrmlManager->GetOutputVolumeMRMLID())
    {
    this->PreviewSelector->SetSelected(
      this->PreviewSelector->GetMRMLScene()->
      GetNodeByID(mrmlManager->GetOutputVolumeMRMLID()));
    }
    
  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->PreviewSelector->GetWidgetName()); 
    
    
   this->AddPreviewGUIObservers();  
    
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
    this->IntensityDistributionHistogramButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->IntensityDistributionHistogramButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->IntensityDistributionHistogramButton->
      SetLabelText("Target Image:");
    this->IntensityDistributionHistogramButton->
      SetBalloonHelpString("Select a target image to adjust intensity distribution");

    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    this->IntensityDistributionHistogramButton->GetWidgetName());
  
  this->PopulateIntensityDistributionTargetVolumeSelector();    
  
  /*if (mrmlManager->GetTargetNumberOfSelectedVolumes() > 0)
    {
    this->IntensityDistributionHistogramButton->GetWidget()->SetValue(mrmlManager->GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(0)));
    }*/
  // Create the histogram
  
  if (!this->IntensityDistributionHistogramHistogram)
    {
    this->IntensityDistributionHistogramHistogram = vtkKWHistogram::New();
    this->IntensityDistributionHistogramHistogramVisu = vtkKWColorTransferFunctionEditor::New();
    this->IntensityDistributionHistogramHistogramFunc = vtkColorTransferFunction::New();
    }
    
   if (!this->IntensityDistributionHistogramHistogramVisu->IsCreated())
    {
    this->IntensityDistributionHistogramHistogramVisu->SetParent(prior_page);
    this->IntensityDistributionHistogramHistogramVisu->Create();
    this->IntensityDistributionHistogramHistogramVisu->SetBorderWidth(2);
    this->IntensityDistributionHistogramHistogramVisu->SetReliefToGroove();
    this->IntensityDistributionHistogramHistogramVisu->SetPadX(2);
    this->IntensityDistributionHistogramHistogramVisu->SetPadY(2);
    this->IntensityDistributionHistogramHistogramVisu->SetPointPositionInValueRangeToTop(); 
    this->IntensityDistributionHistogramHistogramVisu->MidPointEntryVisibilityOn();
    //this->IntensityDistributionHistogramHistogramVisu->PointGuidelineVisibilityOn();
    this->IntensityDistributionHistogramHistogramVisu->MidPointGuidelineVisibilityOn(); 
    this->IntensityDistributionHistogramHistogramVisu->SetLabelPositionToTop();
    }
     
         
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2", 
    this->IntensityDistributionHistogramHistogramVisu->GetWidgetName());
    
    vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
    this->IntensityDistributionHistogramButton->SetEnabled(
    mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->IntensityDistributionHistogramButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int vol_id = mrmlManager->GetTargetSelectedVolumeNthID(0);
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
    this->ClassAndNodeList->SetPotentialCellColorsChangedCommand(this->ClassAndNodeList,"ScheduleRefreshColorsOfAllCellsWithWindowCommand");
    //this->ClassAndNodeList->SetWidth(0);
    
    int col_index;
    
    col_index = this->ClassAndNodeList->AddColumn("Class");
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
      
  int i = 0;
  int j = 0;
  
  //const char* maintin[200];
  
  double node_value[6];
  
  for(j = 0;j < this->nbOfLeaf;j++){
  this->leafName[j] = mrmlManager->GetTreeNodeName(this->leafID[j]);//sans this avant
  this->orderedLabel[j] = mrmlManager->GetTreeNodeIntensityLabel(this->leafID[j]);
  std::cout<<"Label: "<<this->orderedLabel[j]<<std::endl;
  }

  double* colors; 

  vtkMRMLScene *mrmlScene   = mrmlManager->GetMRMLScene();
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(mrmlScene->GetNodeByID( mrmlManager->GetColormap() ));
  
  for(i=0; i < this->nbOfLeaf; i++)
  {
  this->ClassAndNodeList->InsertCellText(i,0,this->leafName[i]);
  this->ClassAndNodeList->SetCellWindowCommandToComboBoxWithValues(i,0,this->nbOfLeaf,leafName);
  this->ClassAndNodeList->InsertCellTextAsInt(i,1,i+1);
  
  
  this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i, node_value);
  
  const char *color[200];
  color[0] = "1.0 0.0 0.0";
  
  colors = colorNode->GetLookupTable()->GetTableValue(mrmlManager->GetTreeNodeIntensityLabel(leafID[i]));

  char red[20];
  double redDouble = colors[0];
  char green[20];
  double greenDouble = colors[1];
  char blue[20];
  double blueDouble = colors[2];
  
  sprintf(red,"%f",redDouble);
  sprintf(green,"%f",greenDouble);
  sprintf(blue,"%f",blueDouble);

  char listColor[11] = {"        "};

  listColor[0] = red[0];
  listColor[1] = red[1];
  listColor[2] = red[2];

  listColor[4] = green[0];
  listColor[5] = green[1];
  listColor[6] = green[2];

  listColor[8] = blue[0];
  listColor[9] = blue[1];
  listColor[10] = blue[2];

  std::cout<<"new color: "<<listColor<<std::endl;

  color[0] = listColor;

  this->ClassAndNodeList->InsertCellText(i,2,color[0]);
  this->ClassAndNodeList->SetCellWindowCommandToColorButton(i,2);
  }
                
    this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2 -pady 2", 
    this->ClassAndNodeList->GetWidgetName());

  this->AddColumnListGUIObservers();

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

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
               this->TestButton->GetWidgetName());
  
  this->AddTestButtonGUIEvents();             


   //////////  END NEW
   
  // Create the EM menu button

  if (!this->StoppingConditionsEMMenuButton)
    {
    this->StoppingConditionsEMMenuButton = vtkKWMenuButtonWithLabel::New();
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
    vtkKWEntry *entry = this->StoppingConditionsEMIterationsEntry->GetWidget();
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
    this->StoppingConditionsMFAMenuButton = vtkKWMenuButtonWithLabel::New();
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
    this->StoppingConditionsMFAIterationsEntry = vtkKWEntryWithLabel::New();
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
    this->StoppingConditionsBiasIterationsEntry = vtkKWEntryWithLabel::New();
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

  this->Script(
    "pack %s -side left -anchor nw -padx 5 -pady {20 2}", 
    this->PrintBasicFrame->GetWidgetName());

  // Create the print weight checkbutton

  if (!this->NodeParametersPrintWeightCheckButton)
    {
    this->NodeParametersPrintWeightCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintWeightCheckButton->IsCreated())
    {
    this->NodeParametersPrintWeightCheckButton->SetParent(this->PrintBasicFrame);
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
    this->NodeParametersPrintQualityCheckButton->SetParent(this->PrintBasicFrame);
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
    this->NodeParametersPrintBiasCheckButton->SetParent(this->PrintBasicFrame);
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
    this->NodeParametersPrintLabelMapCheckButton->SetParent(this->PrintBasicFrame);
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
    this->PrintConvergenceFrame->SetLabelText(
      "Convergence");
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

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1", 
               this->NodeParametersPrintEMLabelMapConvergenceCheckButton->GetWidgetName());

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

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1", 
               this->NodeParametersPrintEMWeightsConvergenceCheckButton->GetWidgetName());

  // Create the print MFA label map convergence checkbutton

  if (!this->NodeParametersPrintMFALabelMapConvergenceCheckButton)
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersPrintMFALabelMapConvergenceCheckButton->IsCreated())
    {
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetParent(
      this->PrintConvergenceFrame->GetFrame());
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->Create();
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetLabelText(
      "MFA Label Map:");
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 1", 
    this->NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidgetName());

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

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 1", 
               this->NodeParametersPrintMFAWeightsConvergenceCheckButton->GetWidgetName());

  // Create the frame

  if (!this->NodeParametersPCAFrame)
    {
    this->NodeParametersPCAFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->NodeParametersPCAFrame->IsCreated())
    {
    this->NodeParametersPCAFrame->SetParent(advanced_page);
    this->NodeParametersPCAFrame->Create();
    this->NodeParametersPCAFrame->SetLabelText(
      "PCA Parameters");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
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

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
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

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->NodeParametersExcludeIncompleteEStepCheckButton->GetWidgetName());


  // Create the Tree node generate background probability check button

  if (!this->NodeParametersGenerateBackgroundProbabilityCheckButton)
    {
    this->NodeParametersGenerateBackgroundProbabilityCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->NodeParametersGenerateBackgroundProbabilityCheckButton->IsCreated())
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
//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  PopulateClassAndNodeList()
{  
  int i;
  
  for(i=1; i<(this->nbOfLeaf+1); i++)
  {
  this->ClassAndNodeList->InsertCellTextAsInt(i-1,1,i);
  }

}

//----------------------------------------------------------------------------
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
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
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
      if (i == 0)
        {
        this->IntensityDistributionHistogramButton->GetWidget()->SetValue(name);
        }
      }
    }
}
//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::
  IntensityDistributionTargetSelectionChangedCallback(vtkIdType target_vol_id)
{
std::cout<<"INTENSITY DISTRIBUTION TARGET SELECTION CHANGED CALLBACK"<<std::endl;
std::cout<<"vol id: "<< target_vol_id <<std::endl;
  
vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(target_vol_id)->GetImageData()->GetPointData()->GetScalars();
this->IntensityDistributionHistogramHistogram->BuildHistogram(array,0);

this->IntensityDistributionHistogramHistogramVisu->SetHistogram(this->IntensityDistributionHistogramHistogram);

double* range = this->IntensityDistributionHistogramHistogram->GetRange();

int size;
size = this->IntensityDistributionHistogramHistogramVisu->GetFunctionSize();
this->IntensityDistributionHistogramHistogramVisu->SetDisableAddAndRemove(0);


if(size > 0){
this->IntensityDistributionHistogramHistogramFunc->RemoveAllPoints();
}

this->IntensityDistributionHistogramHistogramFunc->SetColorSpaceToHSV();
this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[0],0.66,1.0,1.0);
this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[1],0.0,1.0,1.0);

double i;
float color;

for(i = 1; i< this->nbOfLeaf - 1 ; i++){

color = 0.66*(1-i/(this->nbOfLeaf -1));

std::cout<<"color: "<<color << std::endl;

this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint((range[0]+range[1])*(i)/(this->nbOfLeaf-1),color,1.0,1.0);
}


this->IntensityDistributionHistogramHistogramFunc->SetColorSpaceToRGB();

double nodeInfo[6];

double *colors;

vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

vtkMRMLScene            *mrmlScene   = mrmlManager->GetMRMLScene();
    
vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(mrmlScene->GetNodeByID( mrmlManager->GetColormap() ));

for(int j = 0 ; j < this->nbOfLeaf ; j++){
this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j,nodeInfo);

colors = colorNode->GetLookupTable()->GetTableValue(mrmlManager->GetTreeNodeIntensityLabel(leafID[j]));
nodeInfo[1] = colors[0];
nodeInfo[2] = colors[1];
nodeInfo[3] = colors[2];
nodeInfo[5] = 1.0;

this->IntensityDistributionHistogramHistogramFunc->SetNodeValue(j,nodeInfo);
}

this->IntensityDistributionHistogramHistogramVisu->SetDisableAddAndRemove(1);

this->IntensityDistributionHistogramHistogramVisu->SetColorTransferFunction(this->IntensityDistributionHistogramHistogramFunc);
this->IntensityDistributionHistogramHistogramVisu->SetWholeParameterRangeToFunctionRange();
this->IntensityDistributionHistogramHistogramVisu->SetVisibleParameterRangeToWholeParameterRange();
this->IntensityDistributionHistogramHistogramVisu->ParameterRangeVisibilityOn();

this->IntensityDistributionHistogramHistogramVisu->ExpandCanvasWidthOn();
this->IntensityDistributionHistogramHistogramVisu->SetCanvasHeight(180);

}
//----------------------------------------------------------------------------
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
      sprintf(
        buffer, "NodeParametersGlobalPriorChangedCallback %d", 
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
      sprintf(buffer, 
              "NodeParametersSpatialPriorWeightChangedCallback %d",
              static_cast<int>(sel_vol_id));
      this->NodeParametersSpatialPriorWeightScale->SetEndCommand(this, buffer);
      this->NodeParametersSpatialPriorWeightScale->SetEntryCommand(
        this, buffer);
      this->NodeParametersSpatialPriorWeightScale->SetValue(
        mrmlManager->GetTreeNodeSpatialPriorWeight(sel_vol_id));
      }
    else
      {
      this->NodeParametersSpatialPriorWeightScale->SetEnabled(0);
      this->NodeParametersSpatialPriorWeightScale->SetEndCommand(NULL, NULL);
      this->NodeParametersSpatialPriorWeightScale->SetEntryCommand(NULL, NULL);
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
      sprintf(buffer, 
              "NodeParametersInputChannelWeightChangedCallback %d",
              static_cast<int>(sel_vol_id));
      list->SetCellUpdatedCommand(this, buffer);
      for (row = 0; row < nb_of_target_volumes; row++)
        {
        list->AddRow();
        int vol_id = mrmlManager->GetTargetSelectedVolumeNthID(row);
        list->SetCellText(row, 0, mrmlManager->GetVolumeName(vol_id));
        list->SetCellTextAsDouble(
          row, 1, 
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
      this->Script("grid %s", this->NodeParametersAlphaScale->GetWidgetName());
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
      this->Script("grid %s",
                   this->StoppingConditionsEMValueEntry->GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",
                   this->StoppingConditionsEMValueEntry->GetWidgetName());
      }
    }

  // Update the MFA menu button

  if (this->StoppingConditionsMFAMenuButton)
    {
    vtkKWMenu *menu = 
      this->StoppingConditionsMFAMenuButton->GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      this->StoppingConditionsMFAMenuButton->SetEnabled(tree->GetEnabled());
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
        mrmlManager->GetTreeNodeStoppingConditionMFAIterations(sel_vol_id));
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
      entry->SetValueAsDouble(
        mrmlManager->GetTreeNodeStoppingConditionMFAValue(sel_vol_id));
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
      this->Script("grid %s",
                   this->StoppingConditionsMFAValueEntry->GetWidgetName());
      }
    else
      {
      this->Script("grid remove %s",
                   this->StoppingConditionsMFAValueEntry->GetWidgetName());
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
      entry->SetValueAsInt(
        mrmlManager->GetTreeNodeBiasCalculationMaxIterations(sel_vol_id));
      this->Script(
        "grid %s",
        this->StoppingConditionsBiasIterationsEntry->GetWidgetName());
      }
    else
      {
      entry->SetCommand(NULL, NULL);
      entry->SetValue("");
      this->Script(
        "grid remove %s",
        this->StoppingConditionsBiasIterationsEntry->GetWidgetName());
      }
    }

  // Update the print weight

  if (this->NodeParametersPrintWeightCheckButton)
    {
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintWeightCheckButton->GetWidget();
    if (has_valid_selection)
      {
      this->NodeParametersPrintWeightCheckButton->SetEnabled(enabled);
      sprintf(buffer, "NodeParametersPrintWeightCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintWeight(sel_vol_id));
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
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintQualityCheckButton->GetWidget();
    if (has_valid_selection && sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintQualityCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintQuality(sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
                   this->NodeParametersPrintQualityCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script(
        "pack forget %s", 
        this->NodeParametersPrintQualityCheckButton->GetWidgetName());
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
      this->NodeParametersPrintFrequencyScale->SetEntryCommand(this, buffer);
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
      this->Script("pack forget %s", 
                   this->NodeParametersPrintBiasCheckButton->GetWidgetName());
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
      cb->SetSelectedState(mrmlManager->GetTreeNodePrintLabelMap(sel_vol_id));
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
        this->NodeParametersPrintLabelMapCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script(
        "pack forget %s", 
        this->NodeParametersPrintLabelMapCheckButton->GetWidgetName());
      }
    }

  // Update the print EM label map

  if (this->NodeParametersPrintEMLabelMapConvergenceCheckButton)
    {
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintEMLabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintEMLabelMapCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodePrintEMLabelMapConvergence(sel_vol_id));
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
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintEMLabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintEMWeightsCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodePrintEMWeightsConvergence(sel_vol_id));
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
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintMFALabelMapCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodePrintMFALabelMapConvergence(sel_vol_id));
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
    vtkKWCheckButton *cb = 
      this->NodeParametersPrintMFALabelMapConvergenceCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "NodeParametersPrintMFAWeightsCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodePrintMFAWeightsConvergence(sel_vol_id));
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
      this->Script(
        "pack forget %s", 
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
      this->Script(
        "pack forget %s", 
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
      this->Script(
        "pack forget %s", 
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
     this->NodeParametersGenerateBackgroundProbabilityCheckButton->GetWidget();
    if (has_valid_selection && !sel_is_leaf_node)
      {
      sprintf(buffer, "GenerateBackgroundProbabilityCallback %d", 
              static_cast<int>(sel_vol_id));
      cb->SetCommand(this, buffer);
      cb->SetSelectedState(
        mrmlManager->GetTreeNodeGenerateBackgroundProbability(sel_vol_id));
      this->Script(
        "pack %s -side top -anchor nw -padx 2 -pady 2", 
        this->NodeParametersGenerateBackgroundProbabilityCheckButton->GetWidgetName());
      }
    else
      {
      cb->SetCommand(NULL, NULL);
      cb->SetSelectedState(0);
      this->Script(
        "pack forget %s", 
        this->NodeParametersGenerateBackgroundProbabilityCheckButton->GetWidgetName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersGlobalPriorChangedCallback(
  vtkIdType sel_vol_id, double value)
{
  // The class probability has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeClassProbability(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersSpatialPriorWeightChangedCallback(
  vtkIdType sel_vol_id, double value)
{
  // The spatial prior weight has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeSpatialPriorWeight(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersInputChannelWeightChangedCallback(
  vtkIdType sel_vol_id, int row, int, const char *value)
{
  // The input channel weight has changed because of user interaction

  double w = atof(value);
  if (w >= 0 && w <= 1.0)
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (mrmlManager)
      {
      mrmlManager->SetTreeNodeInputChannelWeight(sel_vol_id, row, atof(value));
      }
    }
  else
    {
    this->DisplaySelectedNodeParametersCallback();
    vtkKWMultiColumnList *list = 
      this->NodeParametersInputChannelWeightsList->GetWidget()->GetWidget();
    list->SeeRow(row);
    }
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMCallback(
  vtkIdType sel_vol_id, int value)
{
  // The EM stopping condition has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && value != mrmlManager->GetTreeNodeStoppingConditionEMType(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionEMType(sel_vol_id, value);
    this->DisplaySelectedNodeParametersCallback();
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMIterationsCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The EM iterations has changed because of user interaction

  int v = (int)abs(atoi(value));
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && v != mrmlManager->GetTreeNodeStoppingConditionEMIterations(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionEMIterations(sel_vol_id, v);
    this->DisplaySelectedNodeParametersCallback(); // in case the value < 0
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsEMValueCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The EM value has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeStoppingConditionEMValue(sel_vol_id, atof(value));
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsMFACallback(
  vtkIdType sel_vol_id, int value)
{
  // The MFA stopping condition has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && value != mrmlManager->GetTreeNodeStoppingConditionMFAType(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAType(sel_vol_id, value);
    this->DisplaySelectedNodeParametersCallback();
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsMFAIterationsCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The MFA iterations has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int v = (int)abs(atoi(value));
  if (v != mrmlManager->GetTreeNodeStoppingConditionMFAIterations(sel_vol_id))
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAIterations(sel_vol_id, v);
    this->DisplaySelectedNodeParametersCallback(); // in case the value < 0
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsMFAValueCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The MFA value has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeStoppingConditionMFAValue(sel_vol_id, atof(value));
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::StoppingConditionsBiasIterationsCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The Bias calculation max iterations value 
  // has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodeBiasCalculationMaxIterations(
      sel_vol_id, atoi(value));
    }
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintQualityCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print quality has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintQuality(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::NodeParametersPrintFrequencyChangedCallback(
  vtkIdType sel_vol_id, int value)
{
  // The print frequency has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetTreeNodePrintFrequency(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
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
  // The print EM label map convergence has changed because of user interaction

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
  // The print MFA label map convergence has changed because of user interaction
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
  // The exclude from incomplete EStep checkbox has changed 
  // because of user interaction

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
  vtkKWWizardWorkflow *wizard_workflow = 
    this->GetGUI()->GetWizardWidget()->GetWizardWorkflow();
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  // make sure that the probability for each set of sibling nodes sums
  // to unity

  if (mrmlManager->GetTreeRootNode() != NULL)
    {
    vtkIdType firstBadTreeID = 
      mrmlManager->GetTreeNodeFirstIDWithChildProbabilityError();
    if (firstBadTreeID >= 0)
      {
      std::stringstream ss;
      ss << "Child probabilities must sum to one for node "
         << mrmlManager->GetTreeNodeName(firstBadTreeID)
         << "; right now they sum to "
         << mrmlManager->GetTreeNodeChildrenSumClassProbability(firstBadTreeID)
         << ".  Please fix before continuing---"
         << "you should edit the \"Global Prior\" fields for the"
         << " children nodes of "
         << mrmlManager->GetTreeNodeName(firstBadTreeID) << ".";
      
      std::string parentNodeName = 
        mrmlManager->GetTreeNodeName(firstBadTreeID);
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
void vtkEMSegmentNodeParametersStep::RightClickOnInputChannelWeightsListCallback(int row, int col, int x, int y)
{
  vtkKWMultiColumnList *list = 
    this->NodeParametersInputChannelWeightsList->GetWidget()->GetWidget();
  list->EditCell(row, col);
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
/*
void vtkEMSegmentNodeParametersStep::ProcessUpdatePriorGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{*/
/*
vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();

      double oldSliceSetting[3];
      oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetValue());
      
      std::cout<<"old setting "<< oldSliceSetting[0] <<std::endl;
*/


/*
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
  vtkIdType root_vol_id = 0;
  vtkIdType root2_vol_id = 0;
  int sel_is_leaf_node = 0;
  int has_valid_selection = tree->HasSelection();
  int tatata;
  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    sel_is_leaf_node = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    root_vol_id = mrmlManager->GetTreeRootNodeID();
    root2_vol_id = mrmlManager->GetTreeNodeParentNodeID(sel_vol_id);
    tatata =mrmlManager->GetTreeNodeNumberOfChildren(sel_vol_id);
    }*/
  /*
  std::cout<<"test1: "<<sel_node<<std::endl;
  std::cout<<"test2: "<<sel_vol_id<<std::endl;
  std::cout<<"test3: "<<sel_is_leaf_node<<std::endl;
  std::cout<<"test4: "<<root_vol_id<<std::endl;
  std::cout<<"test5: "<<root2_vol_id<<std::endl;
  std::cout<<"test6: "<<tatata<<std::endl;*/
  /*
  int i;
  int nbfils[mrmlManager->GetTreeNodeNumberOfChildren(sel_vol_id)];
  
    for(i = 0; i < mrmlManager->GetTreeNodeNumberOfChildren(sel_vol_id);i++)
  {
    std::cout<<mrmlManager->GetTreeNodeChildNodeID(sel_vol_id, i)<<std::endl;
    nbfils[i] = mrmlManager->GetTreeNodeNumberOfChildren(mrmlManager->GetTreeNodeChildNodeID(sel_vol_id, i));
  }
  
  *//*
  int enabled = tree->GetEnabled();
  int row;
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];
  */
 // int number = this->MRMLManager->GetTargetNumberOfSelectedVolumes();
 // std::cout<<"test: "<<number<<std::endl;
/*
  // Update the class probability scale
  
  if (this->NodeParametersGlobalPriorScale)
    {
    if (has_valid_selection)
      {
      this->NodeParametersGlobalPriorScale->SetEnabled(enabled);
      sprintf(
        buffer, "NodeParametersGlobalPriorChangedCallback %d", 
        static_cast<int>(sel_vol_id));
      this->NodeParametersGlobalPriorScale->SetEndCommand(this, buffer);
      this->NodeParametersGlobalPriorScale->SetEntryCommand(this, buffer);
      this->NodeParametersGlobalPriorScale->SetValue(0.15);
       //rmlManager->GetTreeNodeClassProbability(sel_vol_id));
      }
    else
      {
      this->NodeParametersGlobalPriorScale->SetEnabled(0);
      this->NodeParametersGlobalPriorScale->SetEndCommand(NULL, NULL);
      this->NodeParametersGlobalPriorScale->SetEntryCommand(NULL, NULL);
      this->NodeParametersGlobalPriorScale->SetValue(0.0);
      }
    }*/
//}
/*
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddUpdatePriorGUIEvents() 
{

    this->UpdatePrior
    ->AddObserver(vtkKWPushButton::InvokedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemoveUpdatePriorGUIEvents()
{
//this->UpdatePrior->RemoveObserver(vtkKWPushButton::InvokedEvent, this->GetGUI()->GetGUICallbackCommand());
}*/

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddPointMovingGUIEvents() 
{

    this->IntensityDistributionHistogramHistogramVisu
    ->AddObserver(vtkKWParameterValueFunctionEditor::PointChangedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemovePointMovingGUIEvents()
{
    this->IntensityDistributionHistogramHistogramVisu
    ->RemoveObservers(vtkKWParameterValueFunctionEditor::PointChangedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessPointMovingGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
if(event == vtkKWParameterValueFunctionEditor::PointChangedEvent){
int id = 2;
double node_value[6];

 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(id, node_value);
 /*
 for(int i=0; i < this->nbOfLeaf; i++)
  {
  this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i, node_value);
  this->ClassAndNodeList->SetCellBackgroundColor(i,2,node_value[1],node_value[2],node_value[3]);
  }*/
  
 this->test();
 
  this->histogramFeedback();
  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddPointAddGUIEvents() 
{

    this->IntensityDistributionHistogramHistogramVisu
    ->AddObserver(vtkKWParameterValueFunctionEditor::PointAddedEvent, this->GetGUI()->GetGUICallbackCommand());
}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemovePointAddGUIEvents()
{

}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessPointAddGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
if(event == vtkKWParameterValueFunctionEditor::PointAddedEvent){

this->size = this->IntensityDistributionHistogramHistogramVisu->GetFunctionSize();

  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::test(){
int i = 0;
int j = 0;
int k = 0;

double midmin;
double midmax;
double node_value_next[6];
double node_value_prev[6];
double node_value[6];
double position[this->nbOfLeaf]; //6



 while(i<(this->nbOfLeaf)){
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i, node_value);
 position[i] = node_value[0];

 i++;
 }

 
 for(j = 1;j<(this->nbOfLeaf)-1;j++){
 midmin = 0;
 midmax = 0;
 
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j-1, node_value_prev);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j, node_value);
 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(j+1, node_value_next);
 
 midmin = node_value[0] - (position[j] - position[j-1])*(1-node_value_prev[4]);
 midmax = node_value_next[0] - (position[j+1] - position[j])*(1-node_value[4]);   
 
 this->class_size[j*2] = midmin;
 this->class_size[j*2+1] = midmax; 

 }


if( this->nbOfLeaf > 2 ){
 this->class_size[0] = position[0];
 this->class_size[1] = this->class_size[2];
 
 this->class_size[(this->nbOfLeaf)*2 - 2] = this->class_size[(this->nbOfLeaf)*2-3];
 this->class_size[(this->nbOfLeaf)*2 - 1] = position[(this->nbOfLeaf)-1];
 
}
else{

midmin = 0;

this->class_size[0] = position[0];
this->class_size[3] = position[1];

 this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(0, node_value);
 
 midmin = position[0] + (position[1] - position[0])*(node_value[4]);
 
this->class_size[1] = midmin;
this->class_size[2] = midmin;

std::cout<<"size: "<< this->class_size[0] <<" to: "<< this->class_size[1] << std::endl;
std::cout<<"size: "<< this->class_size[2] <<" to: "<< this->class_size[3] << std::endl;

}
 
for(k=0;k<this->nbOfLeaf;k++){
 
this->classSize[2*k] = this->class_size[2*k];
this->classSize[2*k+1] = this->class_size[2*k+1];

} 

}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetNumberOfLeaf(
  const char *parent, vtkIdType vol_id)
{
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  
  if(nb_children == 0){
  
  this->nbOfLeaf = this->nbOfLeaf +1;
  this->leafID[this->nbOfLeaf-1] = static_cast<int>(vol_id);
  std::cout <<"ROOT ID: "<< mrmlManager->GetTreeRootNodeID()<< std::endl;
  }
  
  for (int i = 0; i < nb_children; i++)
    {
    this->GetNumberOfLeaf(
      NULL, mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
      }
}
//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetParentPercent(
  int i, vtkIdType vol_id)
{
  this->depth = this->depth +1 ;
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  vtkIdType parent_ID = mrmlManager->GetTreeNodeParentNodeID(vol_id);
  
  
  if(parent_ID != mrmlManager->GetTreeRootNodeID()){
  
  this->classPercentOrder[this->depth][i] = parent_ID;
  this->classPercentOrderCP[this->depth][i] = parent_ID;
  this->GetParentPercent(i, parent_ID);
  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessTestButtonGUIEvents(vtkObject
    *caller, unsigned long event, void *callData)
{
  if (event == vtkKWPushButton::InvokedEvent && caller == this->TestButton)
  {
    for (int i=0; i < this->nbOfLeaf; i++)
    {
      this->depth = 0;
      this->classPercentOrder[0][i]   = this->leafID[i];
      this->classPercentOrderCP[0][i] = this->leafID[i];
      this->GetParentPercent(i,this->leafID[i]);
    }

    int control = 1;
    int position = 0;
    int positionF;

    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

    for (int i=0;i<this->nbOfLeaf;i++)
    {
      for (int j=0; j<200; j++)
      {
        for (int k=0; k<this->nbOfLeaf; k++)
        {
          for (int l=0; l<200; l++)
          {
            if (this->classPercentOrder[j][i] ==
                this->classPercentOrderCP[l][k] &&
                this->classPercentOrder[j][i] != 0)
            {
              this->classPercentOrderCP[l][k] = 0;
              for(int m=0; m < 200 ;m++)
              {
                if(this->correspondanceArray[m] ==
                    this->classPercentOrder[j][i])
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

              this->correspondanceArray[positionF] =
                this->classPercentOrder[j][i];

              for(int z=0; z < this->nbOfLeaf; z++)
              {
                if (strcmp(this->ClassAndNodeList->GetCellText(z,0),
                      mrmlManager->GetTreeNodeName(
                        this->classPercentOrder[0][k])) == 0)
                {
                  this->class_weight[positionF] += this->GetWeight(z);
                }
              }

              control = 1;
            }
          }
        }
      }
    }

    //for(int r = 0;r<200;r++)
    //{
      //std::cout<<this->correspondanceArray[r]<<std::endl;
    //}

    //std::cout<<"in Process GUI"<<std::endl;

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
  this->TestButton->RemoveObservers(vtkKWPushButton::InvokedEvent,this->
      GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::GetPercent(int j, vtkIdType vol_id)
{
  this->depth += 1;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  double weight = 0.0;

  if(nb_children > 0)
  {
    for (int i=0; i < nb_children; i++)
    {
      for(int m=0; m < 200 ;m++)
      {
        if (this->correspondanceArray[m] == mrmlManager->
            GetTreeNodeChildNodeID(vol_id, i))
        {
          weight += this->class_weight[m];
        }
      }
    }

    for (int i=0; i < nb_children; i++)
    {
      for (int m=0; m < 200; m++)
      {
        if (this->correspondanceArray[m] == mrmlManager->
            GetTreeNodeChildNodeID(vol_id, i))
        {
          this->class_weight[m] = (this->class_weight[m])/weight;
        }
      }
    }

    for (int i=0; i < nb_children; i++)
    {
      this->GetPercent(3,mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
    }

    //std::cout<<"in Get Percent"<<std::endl;
    for (int i=0; i < 200; i++)
    {
      if (this->correspondanceArray[i] !=  0 && mrmlManager->GetTreeNode(
            this->correspondanceArray[i]))
      {
        mrmlManager->SetTreeNodeClassProbability(this->correspondanceArray[i],
            this->class_weight[i]);
      }
    }
  }
}

//--------------------------------------------------------------------------
double vtkEMSegmentNodeParametersStep::GetWeight(int z)
{
  double sum = 0.0;

  int start = (int)(this->class_size[z*2]   + 1.5);
  int stop  = (int)(this->class_size[z*2+1] + 0.5);

  for (int i = start; i < stop; i++)
  {
    sum += this->IntensityDistributionHistogramHistogram->GetOccurenceAtValue(i);
  }

  return sum;
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddPreviewGUIObservers()
{
  this->PreviewSelector->AddObserver(vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent,this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemovePreviewGUIObservers()
{
  this->PreviewSelector->RemoveObservers(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
    this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessPreviewGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData)
{
  if (caller == this->PreviewSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
      this->PreviewSelector->GetSelected() != NULL)
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (mrmlManager)
      {
      mrmlManager->SetOutputVolumeMRMLID(
        this->PreviewSelector->GetSelected()->GetID());
      }
      
      // find output volume
  if (!mrmlManager->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  vtkMRMLScalarVolumeNode *outVolume = 
    mrmlManager->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found---aborting segmentation.");
    return;
    }
     std::cout<<"process preview gui event"<<std::endl;
    }
}
//--------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::histogramFeedback()
{

std::cout<<"in histogramFeedback"<<std::endl;

vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();  
// find volumes
/*

      // find input volume
  if (!mrmlManager->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  vtkMRMLVolumeNode *inVolume  = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0);
  if (inVolume == NULL)
    {
    vtkErrorMacro("Can't get first target image.");
    return;
    }
    
      // find output volume
  if (!mrmlManager->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  vtkMRMLScalarVolumeNode *outVolume = 
    mrmlManager->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found---aborting segmentation.");
    return;
    }

  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());
  
  
  this->PREVIEW = vtkImageData::New(); 
  
  vtkTransform* xyToijk = vtkTransform::New();

  xyToijk  = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetXYToIJKTransform();

  vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->GetSliceNode();
  unsigned int dimensions[3];
  snode->GetDimensions(dimensions);

  std::cout<<"slice size: "<<dimensions[0]<<" "<<dimensions[1]<<std::endl;

  double dim0;
  double dim1;

  dim0 = dimensions[0];
  dim1 = dimensions[1];
  
  //GET SIZE OF THE ARRAY TO BE PROCESSED AND THE FIRST PIXEL IN FRAME
  
  double size1 = 0;
  double size2 = 0;
  double xyPt[4];
  double ijkPt[3];
  double begin[2];
  
  xyPt[1] = round(dim1/2);
  xyPt[2] = 0;
  xyPt[3] = 1;
  
  int* extent  = inVolume->GetImageData()->GetWholeExtent();
  
  for(int i = 0; i < dim0; i++) {       
  xyPt[0] = round(i);
  xyToijk->MultiplyPoint(xyPt,ijkPt);
  
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size1 == 0){
  begin[0] = i;
  }
  size1++;
  }
  }
  
  
  
  xyPt[0] = round(dim0/2);
  
  for(int i = 0; i < dim1; i++) {       
        xyPt[1] = round(i);
        xyToijk->MultiplyPoint(xyPt,ijkPt);
  //STORAGE->SetScalarComponentFromDouble(ijkPt[0],ijkPt[1],ijkPt[2],0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt[0],ijkPt[1],ijkPt[2],0));
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size2 == 0){
  begin[1] = i;
  }
  size2++;
  }
  }
 
 // GET BOUNDS OF THE ARRAY IN IJK
 int size[6];
 
 xyPt[0] = size1;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;

 double ijkPt1[3];
 double ijkPt2[3];
 double ijkPt3[3];
 
 xyToijk->MultiplyPoint(xyPt,ijkPt1);
  
 xyPt[0] = 0;
 xyPt[1] = size2;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 xyToijk->MultiplyPoint(xyPt,ijkPt2);
 
 xyPt[0] = 0;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 xyToijk->MultiplyPoint(xyPt,ijkPt3);
 
 
 size[0] = sqrt((ijkPt1[0]-ijkPt3[0])*(ijkPt1[0]-ijkPt3[0]));
 size[1] = sqrt((ijkPt2[0]-ijkPt3[0])*(ijkPt2[0]-ijkPt3[0]));
 size[2] = sqrt((ijkPt1[1]-ijkPt3[1])*(ijkPt1[1]-ijkPt3[1]));
 size[3] = sqrt((ijkPt2[1]-ijkPt3[1])*(ijkPt2[1]-ijkPt3[1]));
 size[4] = sqrt((ijkPt1[2]-ijkPt3[2])*(ijkPt1[2]-ijkPt3[2]));
 size[5] = sqrt((ijkPt2[2]-ijkPt3[2])*(ijkPt2[2]-ijkPt3[2]));
 
 std::cout<<"SIZE 1: "<< size[0] <<std::endl;
 std::cout<<"SIZE 1: "<< size[1] <<std::endl;
 std::cout<<"SIZE 2: "<< size[2] <<std::endl;
 std::cout<<"SIZE 2: "<< size[3] <<std::endl;
 std::cout<<"SIZE 3: "<< size[4] <<std::endl;
 std::cout<<"SIZE 3: "<< size[5] <<std::endl;
 
 int compt = 0;
 int pos[2];
 for(int i = 0; i < 6; i++){
 if (size[i] > 0)
 {
 size[compt] = size[i];
 if(i<2)
 pos[compt] = 0;
 if(i>1 && i<4)
 pos[compt] = 1;
 if(i>3)
 pos[compt] = 2;
 compt ++;
 }
 }

double populateXY[4];
double populateIJK[4];

populateXY[2] = 0;
populateXY[3] = 1;

//GET EVOLUTION TO POPULATE (POSITION INCREMENT OR DECREMENT)
double direction[4][2];
int start = begin[0]+begin[1];
for(int i = begin[0]; i < begin[0]+2; i++) {
for(int j = begin[1]; j < begin[1]+2; j++) {
populateXY[0] = i;
populateXY[1] = j;
xyToijk->MultiplyPoint(populateXY,populateIJK);

direction[i+j-start][0] = populateIJK[pos[0]];
direction[i+j-start][1] = populateIJK[pos[1]];
}
}

double evolution[2];

if(direction[0][0] == direction [1][0]){
if(direction[0][0]-direction[2][0] < 0){
evolution[0] = 1;
}
else{
evolution[0] = 0;
}
}
else{
if(direction[0][0]-direction[1][0] < 0){
evolution[0] = 1;
}
else{
evolution[0] = 0;
}
}

if(direction[0][1] == direction [1][1]){
if(direction[0][1]-direction[2][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}
else{
if(direction[0][1]-direction[1][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}

 xyPt[0] = begin[0];
 xyPt[1] = begin[1];
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 double originIJK[3];
 
 xyToijk->MultiplyPoint(xyPt,originIJK);
 
// POPULATE THE CORRESPONDING ARRAY AND MASK

    std::cout<<"origin IJK: "<< originIJK[0]<< " "<< originIJK[1] << " "<< originIJK[2] <<std::endl;
    std::cout<<"size IJK: "<< size[0]<< " "<< size[1] <<std::endl;

if(pos[0]==0 && pos[1] == 1){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j-1,originIJK[1]-i,originIJK[2],0,12.0);
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j-1,originIJK[1]+i,originIJK[2],0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]-i,originIJK[2],0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]+i,originIJK[2],0,12.0);
}

}

if(pos[0]==1 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]-i,0,12.0);
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]+i,0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j+1,originIJK[2]+i,0,12.0);
}

}

if(pos[0]==0 && pos[1] == 2){
if(evolution[0] == 0 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0,12.0);
}
if(evolution[0] == 0 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 0){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0,12.0);
}
if(evolution[0] == 1 && evolution[1] == 1){
for (int j=0;j<size[0];j++) 
for (int i=0;i<size[1];i++)     
this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1],originIJK[2]+i,0,12.0);
}

}

outVolume->SetAndObserveImageData(this->PREVIEW);
outVolume->SetModifiedSinceRead(1);*/
  
  // find input volume
  //scalarvolumenode
    vtkMRMLVolumeNode *inVolume = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetLayerVolumeNode (0);
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

 // create output volume for preview
  vtkMRMLScalarVolumeNode *outVolume =  mrmlManager->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found");
    return;
    }


  vtkTransform* xyToijk = vtkTransform::New();

  xyToijk  = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetBackgroundLayer()->GetXYToIJKTransform();

  vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->GetSliceNode();
  unsigned int dimensions[3];
  snode->GetDimensions(dimensions);

  double dim0 = dimensions[0];
  double dim1 = dimensions[1];
  
  std::cout<<"Dimensions of the red slice"<<std::endl;
  std::cout<<"X: "<<dim0<<" Y: "<<dim1<<std::endl;
  
  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName(name.c_str());
  
  //this->STORAGE = vtkImageData::New(); 
   

  //stoVolume->SetAndObserveImageData(this->PREVIEW);
  
  //GET SIZE OF THE ARRAY TO BE PROCESSED AND THE FIRST PIXEL IN FRAME
  
  double size1 = 0;
  double size2 = 0;
  double xyPt[4];
  double ijkPt[3];
  int begin[2];
  
  xyPt[1] = round(dim1/2);
  xyPt[2] = 0;
  xyPt[3] = 1;
  
  int* extent  = inVolume->GetImageData()->GetWholeExtent();
  
  for(int i = 0; i < dim0; i++) {       
  xyPt[0] = round(i);
  xyToijk->MultiplyPoint(xyPt,ijkPt);
  
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size1 == 0){
  begin[0] = i;
  }
  size1++;
  }
  }
  
  
  
  xyPt[0] = round(dim0/2);
  
  for(int i = 0; i < dim1; i++) {       
        xyPt[1] = round(i);
        xyToijk->MultiplyPoint(xyPt,ijkPt);
  //STORAGE->SetScalarComponentFromDouble(ijkPt[0],ijkPt[1],ijkPt[2],0,inVolume->GetImageData()->GetScalarComponentAsDouble(ijkPt[0],ijkPt[1],ijkPt[2],0));
  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 || ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] ){
  //std::cout<<"OUT OF VOI"<<std::endl;
  }
  else{
  if(size2 == 0){
  begin[1] = i;
  }
  size2++;
  }
  }
  //std::cout<<"DIM : "<< dim0 <<" SIZE : "<< size1 <<std::endl;
  //std::cout<<"DIM : "<< dim1 <<" SIZE : "<< size2 <<std::endl;
  
  std::cout<<"Extent of the input volume: "<< extent[0]<<" " <<extent[1]<<" "<<extent[2]<<" "<<extent[3]<<" "<<extent[4]<<" "<<extent[5]<<std::endl;
  std::cout<<"Size of the real slice XY: "<< size1<<" " <<size2<<std::endl;
  std::cout<<"Beginning of the real slice XY: "<< begin[1]<<" " <<begin[2]<<std::endl;
  
 //stoVolume->SetModifiedSinceRead(1);
 
 // GET BOUNDS OF THE ARRAY IN IJK
 int size[6];
 
 xyPt[0] = size1;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;

 double ijkPt1[3];
 double ijkPt2[3];
 double ijkPt3[3];
 
 xyToijk->MultiplyPoint(xyPt,ijkPt1);
  
 xyPt[0] = 0;
 xyPt[1] = size2;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 xyToijk->MultiplyPoint(xyPt,ijkPt2);
 
 xyPt[0] = 0;
 xyPt[1] = 0;
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 xyToijk->MultiplyPoint(xyPt,ijkPt3);
 
 
 size[0] = (int)(sqrt((ijkPt1[0]-ijkPt3[0])*(ijkPt1[0]-ijkPt3[0]))+0.5);
 size[1] = (int)(sqrt((ijkPt2[0]-ijkPt3[0])*(ijkPt2[0]-ijkPt3[0]))+0.5);
 size[2] = (int)(sqrt((ijkPt1[1]-ijkPt3[1])*(ijkPt1[1]-ijkPt3[1]))+0.5);
 size[3] = (int)(sqrt((ijkPt2[1]-ijkPt3[1])*(ijkPt2[1]-ijkPt3[1]))+0.5);
 size[4] = (int)(sqrt((ijkPt1[2]-ijkPt3[2])*(ijkPt1[2]-ijkPt3[2]))+0.5);
 size[5] = (int)(sqrt((ijkPt2[2]-ijkPt3[2])*(ijkPt2[2]-ijkPt3[2]))+0.5);
 

 /*std::cout<<"SIZE 2: "<< size[2] <<std::endl;
 std::cout<<"SIZE 2: "<< size[3] <<std::endl;
 std::cout<<"SIZE 3: "<< size[4] <<std::endl;
 std::cout<<"SIZE 3: "<< size[5] <<std::endl;*/
 
 int compt = 0;
 int pos[2];
 for(int i = 0; i < 6; i++){
 if (size[i] > 0)
 {
 size[compt] = size[i];
 if(i<2)
 pos[compt] = 0;
 if(i>1 && i<4)
 pos[compt] = 1;
 if(i>3)
 pos[compt] = 2;
 compt ++;
 }
 }
 
 /*std::cout<<"SIZE OF THE ARRAY TO BE PROCESSED"<<std::endl;
 std::cout<<"SIZE : "<< size[0] <<" POSITION: "<< pos[0] <<std::endl;
 std::cout<<"SIZE : "<< size[1] <<" POSITION: "<< pos[1] <<std::endl;*/
 
double populateXY[4];
double populateIJK[4];

populateXY[2] = 0;
populateXY[3] = 1;

/*
for(int i = begin[0]; i < begin[0]+size1; i++) {
for(int j = begin[1]; j < begin[1]+size2; i++) {
populateXY[0] = i;
populateXY[1] = j;

xyToijk->MultiplyPoint(populateXY,populateIJK);

outStorage->SetComponent(i*(size[0])+j,0,-50);
}
}
*/

//GET EVOLUTION TO POPULATE (POSITION INCREMENT OR DECREMENT)
double direction[4][2];
int start = begin[0]+begin[1];
for(int i = begin[0]; i < begin[0]+2; i++) {
for(int j = begin[1]; j < begin[1]+2; j++) {
populateXY[0] = i;
populateXY[1] = j;
xyToijk->MultiplyPoint(populateXY,populateIJK);
/*std::cout<<"TEST X: "<<populateIJK[pos[0]]<<std::endl;
std::cout<<"TEST Y: "<<populateIJK[pos[1]]<<std::endl;*/
direction[i+j-start][0] = populateIJK[pos[0]];
direction[i+j-start][1] = populateIJK[pos[1]];
}
}

/*
std::cout<<"DIRECTIONS"<<std::endl;
std::cout<< direction[0][0] << " " << direction[0][1] <<std::endl;
std::cout<< direction[1][0] <<" " <<  direction[1][1] <<std::endl;
std::cout<< direction[2][0] << " " << direction[2][1] <<std::endl;
std::cout<< direction[3][0] << " " << direction[3][1] <<std::endl;*/

double evolution[2];

if(direction[0][0] == direction [1][0]){
  if(direction[0][0]-direction[2][0] < 0){
    evolution[0] = 1;
  }
  else{
    evolution[0] = 0;
  }
}
else{
if(direction[0][0]-direction[1][0] < 0){
evolution[0] = 1;
}
else{
evolution[0] = 0;
}
}

if(direction[0][1] == direction [1][1]){
if(direction[0][1]-direction[2][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}
else{
if(direction[0][1]-direction[1][1] < 0){
evolution[1] = 1;
}
else{
evolution[1] = 0;
}
}

 xyPt[0] = begin[0];
 xyPt[1] = begin[1];
 xyPt[2] = 0;
 xyPt[3] = 1;
 
 int done = 0;
 double originIJK[3];
 
 xyToijk->MultiplyPoint(xyPt,originIJK);
 
 //std::cout << "infos"<<std::endl;
 std::cout<<"slice size IJK: "<<size[0]<<" "<<size[1]<<std::endl;
 std::cout << "position: "<< pos[0]<<" " << pos[1] << std::endl;
 std::cout << "evolution: "<<evolution[0]<<" " << evolution[1] << std::endl;

//for(int k = 0; k<this->nbOfLeaf;k++){ 
 //std::cout << "leaf: "<< k << " label: " << this->orderedLabel[k] << std::endl;
 //std::cout << "class size: " << this->class_size[2*k] <<" to: "<< this->class_size[2*k+1]<<std::endl;
 //std::cout << "get occurence: "<< this->IntensityDistributionHistogramHistogram->GetOccurenceAtValue(this->class_size[2*k])<<std::endl;

  //}

std::cout<<"origin: "<< originIJK[0] <<" " << originIJK[1] <<" "  <<originIJK[2] <<std::endl;

int iMin, iMax, jMin, jMax, kMin, kMax;

if(pos[0]==0 && pos[1] == 1){
  kMin = (int)(originIJK[2]+0.5);
  kMax = (int)(originIJK[2]+0.5);
  if(evolution[0] == 0 && evolution[1] == 0){
    iMin = (int)(originIJK[0]-size[0]+0.5);
    iMax = (int)(originIJK[0]+0.5);
    jMin = (int)(originIJK[1]-size[1]+0.5);
    jMax = (int)(originIJK[1]+0.5);
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    iMin = (int)(originIJK[0]-size[0]+0.5);
    iMax = (int)(originIJK[0]+0.5);
    jMin = (int)(originIJK[1]+0.5);
    jMax = (int)(originIJK[1]+size[1]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    iMin = (int)(originIJK[0]+0.5);
    iMax = (int)(originIJK[0]+size[0]+0.5);
    jMin = (int)(originIJK[1]-size[1]+0.5);
    jMax = (int)(originIJK[1]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    iMin = (int)(originIJK[0]+0.5);
    iMax = (int)(originIJK[0]+size[0]+0.5);
    jMin = (int)(originIJK[1]+0.5);
    jMax = (int)(originIJK[1]+size[1]+0.5);
  }
}

if(pos[0]==0 && pos[1] == 2){
  jMin = (int)(originIJK[1]+0.5);
  jMax = (int)(originIJK[1]+0.5);
  if(evolution[0] == 0 && evolution[1] == 0){
    iMin = (int)(originIJK[0]-size[0]+0.5);
    iMax = (int)(originIJK[0]+0.5);
    kMin = (int)(originIJK[2]-size[1]+0.5);
    kMax = (int)(originIJK[2]+0.5);
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    iMin = (int)(originIJK[0]-size[0]+0.5);
    iMax = (int)(originIJK[0]+0.5);
    kMin = (int)(originIJK[2]+0.5);
    kMax = (int)(originIJK[2]+size[1]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    iMin = (int)(originIJK[0]+0.5);
    iMax = (int)(originIJK[0]+size[0]+0.5);
    kMin = (int)(originIJK[2]-size[1]+0.5);
    kMax = (int)(originIJK[2]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    iMin = (int)(originIJK[0]+0.5);
    iMax = (int)(originIJK[0]+size[0]+0.5);
    kMin = (int)(originIJK[2]+0.5);
    kMax = (int)(originIJK[2]+size[1]+0.5);
  }
}

if(pos[0]==1 && pos[1] == 2){
  iMin = (int)(originIJK[0]+0.5);
  iMax = (int)(originIJK[0]+0.5);
  if(evolution[0] == 0 && evolution[1] == 0){
    kMin = (int)(originIJK[2]-size[1]+0.5);
    kMax = (int)(originIJK[2]+0.5);
    jMin = (int)(originIJK[1]-size[0]+0.5);
    jMax = (int)(originIJK[1]+0.5);
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    kMin = (int)(originIJK[2]+0.5);
    kMax = (int)(originIJK[2]+size[1]+0.5);
    jMin = (int)(originIJK[1]-size[0]+0.5);
    jMax = (int)(originIJK[1]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    kMin = (int)(originIJK[2]-size[1]+0.5);
    kMax = (int)(originIJK[2]+0.5);
    jMin = (int)(originIJK[1]+0.5);
    jMax = (int)(originIJK[1]+size[0]+0.5);
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    kMin = (int)(originIJK[2]+0.5);
    kMax = (int)(originIJK[2]+size[1]+0.5);
    jMin = (int)(originIJK[1]+0.5);
    jMax = (int)(originIJK[1]+size[0]+0.5);
  }
}

/*
if(pos[0]==0 && pos[1] == 1){
  if(evolution[0] == 0 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]-j+1,originIJK[1]-i+1,originIJK[2],0);// originIJK[1]-i,originIJK[2],0);      
        for (int k=0;k<this->nbOfLeaf;k++){
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j+1,originIJK[1]-i+1,originIJK[2],0,pixelValue);
      }
    }
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
         double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]-j-1,originIJK[1]+i,originIJK[2],0);      
        for (int k=0;k<this->nbOfLeaf;k++){ 
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j-1,originIJK[1]+i,originIJK[2],0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = 0;
        pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]+j+1,originIJK[1]-i,originIJK[2],0);     
          for (int k=0;k<this->nbOfLeaf;k++){
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]-i,originIJK[2],0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]+j+1,originIJK[1]+i,originIJK[2],0);      
        for (int k=0;k<this->nbOfLeaf;k++){  
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1]+i,originIJK[2],0,pixelValue);
      }
    }
  }
}

if(pos[0]==1 && pos[1] == 2){
  if(evolution[0] == 0 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j+1,originIJK[2]-i+1,0);//originIJK[1]-j-1,originIJK[2]-i,0);
        for (int k=0;k<this->nbOfLeaf;k++){ 
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j+1,originIJK[2]-i+1,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]+i,0);      
        for (int k=0;k<this->nbOfLeaf;k++){  
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]-j-1,originIJK[2]+i,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0);     
        for (int k=0;k<this->nbOfLeaf;k++){
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j,originIJK[2]-i,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0],originIJK[1]+j+1,originIJK[2]+i,0);     
        for (int k=0;k<this->nbOfLeaf;k++){  
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0],originIJK[1]+j+1,originIJK[2]+i,0,pixelValue);
      }
    }
  }
}

if(pos[0]==0 && pos[1] == 2){
  if(evolution[0] == 0 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0);     
        //for (int k=0;k<this->nbOfLeaf;k++){
        //  if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
        //    pixelValue = this->orderedLabel[k];
        //    done = 1;
        //  }
       // }
       // done = 0;
      this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]-i,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 0 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0);   
        //for (int k=0;k<this->nbOfLeaf;k++){
        //  if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
        //    pixelValue = this->orderedLabel[k];
        //    done = 1;
        // }
        //}
        //done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]-j,originIJK[1],originIJK[2]+i,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 0){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0);     
        //for (int k=0;k<this->nbOfLeaf;k++){   
        //  if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
        //    pixelValue = this->orderedLabel[k];
        //    done = 1;
        //  }
        //}
        //done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j,originIJK[1],originIJK[2]-i,0,pixelValue);
      }
    }
  }
  if(evolution[0] == 1 && evolution[1] == 1){
    for (int j=0;j<size[0]+1;j++){ 
      for (int i=0;i<size[1]+1;i++){
        double pixelValue = this->PREVIEW->GetScalarComponentAsDouble(originIJK[0]+j+1,originIJK[1],originIJK[2]+i,0);     
        for (int k=0;k<this->nbOfLeaf;k++){ 
          if((this->class_size[2*k] <= pixelValue ) && (this->class_size[2*k+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[k];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(originIJK[0]+j+1,originIJK[1],originIJK[2]+i,0,pixelValue);
      }
    }
  }
}*/

/*
// Matrices for axial, coronal, sagittal, oblique view orientations
  static double axialElements[16] = {
           1, 0, 0, 0,
           0, 1, 0, 0,
           0, 0, 1, 0,
           0, 0, 0, 1 };

  static double coronalElements[16] = {
           1, 0, 0, 0,
           0, 0, 1, 0,
           0,-1, 0, 0,
           0, 0, 0, 1 };

  static double sagittalElements[16] = {
           0, 0,-1, 0,
           1, 0, 0, 0,
           0,-1, 0, 0,
           0, 0, 0, 1 };

// Set the slice orientation
  vtkMatrix4x4 *resliceAxes = vtkMatrix4x4::New();
  resliceAxes->DeepCopy(axialElements); 
// Set the point through which to slice
  resliceAxes->SetElement(0, 3, 100);
  resliceAxes->SetElement(1, 3, 100);
  resliceAxes->SetElement(2, 3, 100);
*/
  this->SliceExtracted = vtkExtractVOI::New();
  this->SliceExtracted->SetInput(inVolume->GetImageData());
  //this->SliceExtracted->SetVOI(originIJK[0]-size[0],originIJK[0],originIJK[1]-size[1],originIJK[1],originIJK[2],originIJK[2]);
  this->SliceExtracted->SetVOI(iMin,iMax,jMin,jMax,kMin,kMax);
  //this->SliceExtracted->ClipDataOn();
  //this->SliceExtracted->SetInputConnection(inVolume->GetImageData());
  //this->SliceExtracted->SetOutputExtent(0,60,0,60,0,0);
  //this->SliceExtracted->SetOutputDimensionality(2);
  //this->SliceExtracted->SetResliceAxes(resliceAxes); 
  ///this->SliceExtracted->SetResliceTransform(xyToijk);
  //this->SliceExtracted->SetOutputOrigin(100.0,100.0,100.0);
  //this->SliceExtracted->SetOutputDimensionality(2);
  this->SliceExtracted->Update(); 
  
  std::cout<<"Boundaries: \n"<<std::endl;
  std::cout<<"iMin: "<<iMin<<"ixMax: "<<iMax<<" jMin: "<<jMin<<" jMax: "<<jMax<<" kMin: "<<kMin<<" kMax: "<<kMax<<std::endl;
  
  //double SliceOrigin[3];
  //this->SliceExtracted->GetResliceAxesOrigin(SliceOrigin);
  
  vtkMatrix4x4 *rasToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *ijkToRAS = vtkMatrix4x4::New();
  inVolume->GetRASToIJKMatrix(rasToIJK);
  inVolume->GetIJKToRASMatrix(ijkToRAS);
  
  double *inRAS = new double[4];
  double *inIJK = new double[4];
  inIJK[0] = iMin;
  inIJK[1] = jMin;
  inIJK[2] = kMin;
  inIJK[3] = 1;
  

  //double *inIJK = new double[4];
  
  //rasToIJK->invert();
  
  ijkToRAS->MultiplyPoint(inIJK,inRAS);
  
  std::cout<<"IJK: "<<inIJK[0]<<"::" <<inIJK[1]<<"::" <<inIJK[2]<<std::endl;
  std::cout<<"RAS: "<<inRAS[0]<<"::" <<inRAS[1]<<"::" <<inRAS[2]<<std::endl;
  
  //std::cout<<"origin0: "<<SliceOrigin[0]<<std::endl;
  //std::cout<<"origin1: "<<SliceOrigin[1]<<std::endl;
  //std::cout<<"origin2: "<<SliceOrigin[2]<<std::endl;
  this->PREVIEW = vtkImageData::New();
  this->PREVIEW->DeepCopy(this->SliceExtracted->GetOutput());
  
  // Labels creation
  
  
  double pixelValue = 0.0;

  for(int i=iMin;i<iMax+1;i++){
    for(int j=jMin;j<jMax+1;j++){
      for(int k=kMin;k<kMax+1;k++){
      //std::cout<<"in loop: "<<std::endl;
        pixelValue = this->PREVIEW->GetScalarComponentAsDouble(i,j,k,0);
        //std::cout<<"pixelValue: "<<pixelValue<<std::endl;     
        for (int l=0;l<this->nbOfLeaf;l++){  
          if((this->class_size[2*l] <= pixelValue ) && (this->class_size[2*l+1] > pixelValue) && !done){
            pixelValue = this->orderedLabel[l];
            done = 1;
          }
        }
        done = 0;
        this->PREVIEW->SetScalarComponentFromDouble(i,j,k,0,pixelValue);   
      }    
    }
  }
  
  vtkImageChangeInformation *sliceInformation = vtkImageChangeInformation::New();
  
  sliceInformation->SetInput(this->PREVIEW);
  sliceInformation->SetOutputExtentStart(0,0,0);

  
  //outVolume->GetDisplayNode()->SetAndObserveColorNodeID(mrmlManager->GetColormap());
  
  outVolume->SetAndObserveImageData(sliceInformation->GetOutput());
  outVolume->LabelMapOn();
  if(outVolume->GetDisplayNode()){
  outVolume->GetDisplayNode()->SetAndObserveColorNodeID(mrmlManager->GetColormap());
  }
  outVolume->SetOrigin(inRAS[0],inRAS[1],inRAS[2]);
  outVolume->SetModifiedSinceRead(1);
    
  this->PREVIEW->Delete();
    
}
//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::AddColumnListGUIObservers() 
{
  this->ClassAndNodeList->AddObserver(
    vtkKWMultiColumnList::CellUpdatedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::RemoveColumnListGUIObservers()
{
  this->ClassAndNodeList->RemoveObservers(
    vtkKWMultiColumnList::CellUpdatedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::ProcessColumnListGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
  if (caller == this->ClassAndNodeList && 
      event == vtkKWMultiColumnList::CellUpdatedEvent) 
    { 
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    //int k = 0;
    
    for(int i = 0; i < this->nbOfLeaf ; i++){
      for(int j = 0; j < this->nbOfLeaf ; j++){
        if(strcmp(this->ClassAndNodeList->GetCellText(i,0),this->leafName[j]) == 0){
        leafIDNewOrder[i] = leafID[j];
        //k++;
        }
      }
    }
    
    std::cout<<"ORDER"<<std::endl;
    double rgb[3];
    double *colors;
    vtkMRMLScene            *mrmlScene   = mrmlManager->GetMRMLScene();
    
    vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(mrmlScene->GetNodeByID( mrmlManager->GetColormap() ));
    
    //colorNode->GetLookupTable()->GetLookupTable->GetTableValue()

double nodeInfo[6];

    for(int i = 0; i < this->nbOfLeaf ; i++){
    //double *colour = NULL;
    //vtkMRMLColorNode *colorNode = mrmlManager->GetNodeByID(leafIDNewOrder[i]);
    //vtkMRMLColorNode *colorNode = mrmlManager->GetColormap();
    std::cout<<"ID: "<<leafIDNewOrder[i]<<std::endl;
    std::cout<<"Label: "<<mrmlManager->GetTreeNodeIntensityLabel(leafIDNewOrder[i])<<std::endl;
    this->orderedLabel[i] = mrmlManager->GetTreeNodeIntensityLabel(leafIDNewOrder[i]);
    std::cout<<"color map: "<<mrmlManager->GetColormap()<<std::endl;
    mrmlManager->GetTreeNodeColor(leafIDNewOrder[i], rgb);
    colors = colorNode->GetLookupTable()->GetTableValue(mrmlManager->GetTreeNodeIntensityLabel(leafIDNewOrder[i]));
    //vtkMRMLEMSTreeNode* n = mrmlManager->GetTreeNode(leafIDNewOrder[i]);
   // n->GetParametersNode()->GetLeafParameterNode->GetColorRGB(rgb);
    //mrmlManager->GetTreeNode(leafIDNewOrder[i])->GetParametersNode()->GetLeafParameterNode->GetColorRGB(rgb);
      
  char red[20];
  double redDouble = colors[0];
  char green[20];
  double greenDouble = colors[1];
  char blue[20];
  double blueDouble = colors[2];
  
  sprintf(red,"%f",redDouble);
  sprintf(green,"%f",greenDouble);
  sprintf(blue,"%f",blueDouble);

  this->IntensityDistributionHistogramHistogramFunc->GetNodeValue(i,nodeInfo);
  nodeInfo[1] = colors[0];
  nodeInfo[2] = colors[1];
  nodeInfo[3] = colors[2];
  this->IntensityDistributionHistogramHistogramFunc->SetNodeValue(i,nodeInfo);
  this->IntensityDistributionHistogramHistogramVisu->Update();

  char listColor[11] = {"        "};

  listColor[0] = red[0];
  listColor[1] = red[1];
  listColor[2] = red[2];

  listColor[4] = green[0];
  listColor[5] = green[1];
  listColor[6] = green[2];

  listColor[8] = blue[0];
  listColor[9] = blue[1];
  listColor[10] = blue[2];

  std::cout<<"new color: "<<listColor<<std::endl;
  const char *color[20];
  color[0] = listColor;

  this->ClassAndNodeList->InsertCellText(i,2,color[0]);
  this->ClassAndNodeList->SetCellWindowCommandToColorButton(i,2);



    }


      if(this->PreviewSelector->GetSelected() != NULL){
    this->histogramFeedback();
}
    //vtkMRMLScene            *mrmlScene   = this->ColorSelectorWidget->GetMRMLScene();
    //mrmlScene->GetNodeByID( mrmlManager->GetColormap() );
    
    //colour = colorNode->GetLookupTable()->GetTableValue(row);
    /*vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (mrmlManager)
      {
      mrmlManager->SetOutputVolumeMRMLID(
        this->PreviewSelector->GetSelected()->GetID());
      }
      
      // find output volume
  if (!mrmlManager->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  vtkMRMLScalarVolumeNode *outVolume = 
    mrmlManager->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found---aborting segmentation.");
    return;
    }
     std::cout<<"process preview gui event"<<std::endl;*/
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentNodeParametersStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemovePointMovingGUIEvents();
  this->RemoveTestButtonGUIEvents();
  this->RemovePreviewGUIObservers();
  this->RemoveColumnListGUIObservers();

  this->ClassAndNodeList->DeleteAllRows();    
}
