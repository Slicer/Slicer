/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentIntensityDistributionsStep.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkEMSegmentIntensityDistributionsStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"
#include "vtkEMSegmentAnatomicalStructureStep.h"

#include "vtkKWApplication.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnListWithScrollbarsWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWParameterValueFunctionInterface.h"
#include "vtkKWParameterValueFunctionEditor.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWSimpleAnimationWidget.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWWindow.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkActor.h"
#include "vtkColorTransferFunction.h"
#include "vtkDataSetMapper.h"
#include "vtkImageData.h"
#include "vtkImageInteractionCallback.h"
#include "vtkImageMapToColors.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageReslice.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkMatrix4x4.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnsignedIntArray.h"
#include "vtkVolume.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkVolumeProperty.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

#include "vtkGaussian2DWidget.h"

#include "vtkSlicerInteractorStyle.h"
#include "vtkSlicerNodeSelectorWidget.h"

#define vtkEMSegmentIntensityDistributionsStep_DebugMacro(msg) \
  std::cout << "\n" << __FILE__ << "\n\tLine:" << __LINE__ \
  << "\t" << msg << std::endl;

#define vtkEMSegmentIntensityDistributionsStep_ErrorMacro(msg) \
  std::cout << "\n" << __FILE__ << "\n\tLine:" << __LINE__ \
  << "\tERROR: " << msg << std::endl; \
  std::cerr << "\n" << __FILE__ << "\n\tLine:" << __LINE__ \
  << "\tERROR: " << msg << std::endl;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityDistributionsStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityDistributionsStep,"$Revision: 1.2$");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::
vtkEMSegmentIntensityDistributionsStep()
{
  this->SetName("6/9. Specify Intensity Distributions");
  this->SetDescription(
    "Define intensity distribution for each anatomical structure.");

  this->IntensityDistributionNotebook                = NULL;
  this->IntensityDistributionSpecificationMenuButton = NULL;
  this->IntensityDistributionMeanMatrix              = NULL;
  this->IntensityDistributionCovarianceMatrix        = NULL;
  this->IntensityDistributionManualSamplingList      = NULL;
  this->ContextMenu                                  = NULL;

  this->IntensityDistributionHistogramFrame          = NULL;
  //this->IntensityDistributionHistogramButton         = NULL;

  this->Gaussian2DWidget                             = NULL;
  this->NumClassesEntryLabel                         = NULL;
  this->Gaussian2DButton                             = NULL;
  //this->Gaussian2DRenderingMenuButton                = NULL;

  this->Gaussian2DVolumeXMenuButton                  = NULL;
  this->Gaussian2DVolumeYMenuButton                  = NULL;

  this->LabelSelector                                = NULL;
  this->LabelmapButton                               = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::
~vtkEMSegmentIntensityDistributionsStep()
{
  if(this->LabelSelector)
  {
    this->LabelSelector->Delete();
    this->LabelSelector = NULL;
  }

  if(this->Gaussian2DVolumeYMenuButton)
  {
    this->Gaussian2DVolumeYMenuButton->Delete();
    this->Gaussian2DVolumeYMenuButton = NULL;
  }

  if(this->Gaussian2DVolumeXMenuButton)
  {
    this->Gaussian2DVolumeXMenuButton->Delete();
    this->Gaussian2DVolumeXMenuButton = NULL;
  }

  if(this->LabelmapButton)
  {
    this->LabelmapButton->Delete();
    this->LabelmapButton = NULL;
  }

  if(this->Gaussian2DButton)
  {
    this->Gaussian2DButton->Delete();
    this->Gaussian2DButton = NULL;
  }

  if(this->NumClassesEntryLabel)
  {
    this->NumClassesEntryLabel->Delete();
    this->NumClassesEntryLabel = NULL;
  }

  /*if(this->IntensityDistributionHistogramButton)
  {
    this->IntensityDistributionHistogramButton->Delete();
    this->IntensityDistributionHistogramButton = NULL;
  }*/

  if(this->IntensityDistributionHistogramFrame)
  {
    this->IntensityDistributionHistogramFrame->Delete();
    this->IntensityDistributionHistogramFrame = NULL;
  }

  if(this->IntensityDistributionNotebook)
  {
    this->IntensityDistributionNotebook->Delete();
    this->IntensityDistributionNotebook = NULL;
  }

  if(this->IntensityDistributionSpecificationMenuButton)
  {
    this->IntensityDistributionSpecificationMenuButton->Delete();
    this->IntensityDistributionSpecificationMenuButton = NULL;
  }

  if(this->IntensityDistributionMeanMatrix)
  {
    this->IntensityDistributionMeanMatrix->Delete();
    this->IntensityDistributionMeanMatrix = NULL;
  }

  if(this->IntensityDistributionCovarianceMatrix)
  {
    this->IntensityDistributionCovarianceMatrix->Delete();
    this->IntensityDistributionCovarianceMatrix = NULL;
  }

  if(this->IntensityDistributionManualSamplingList)
  {
    this->IntensityDistributionManualSamplingList->Delete();
    this->IntensityDistributionManualSamplingList = NULL;
  }

  if(this->ContextMenu)
  {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
  }

  if(this->Gaussian2DWidget)
  {
    this->Gaussian2DWidget->Delete();
    this->Gaussian2DWidget = NULL;
  }

 /* if(this->Gaussian2DRenderingMenuButton)
  {
    this->Gaussian2DRenderingMenuButton->Delete();
    this->Gaussian2DRenderingMenuButton = NULL;
  }*/
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ShowUserInterface()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro("ShowUserInterface");

  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  wizardWidget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anatomicalStep =
    this->GetGUI()->GetAnatomicalStructureStep();
  anatomicalStep->ShowAnatomicalStructureTree();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
  {
    return;
  }

  vtkIdType volId = mrmlManager->GetTreeRootNodeID();

  const char *rootNode = anatomicalStep->GetAnatomicalStructureTree()->
    GetWidget()->FindNodeWithUserDataAsInt(NULL,volId);

  if(rootNode && *rootNode)
  {
    anatomicalStep->SetAnatomicalTreeParentNodeSelectableState(rootNode, 0);
  }

  this->AddManualIntensitySamplingGUIObservers();

  // Override the tree callbacks for that specific step

  anatomicalStep->GetAnatomicalStructureTree()->GetWidget()->
    SetSelectionChangedCommand(this,
      "DisplaySelectedNodeIntensityDistributionsCallback");

  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // Create the notebook

  if(!this->IntensityDistributionNotebook)
  {
    this->IntensityDistributionNotebook = vtkKWNotebook::New();
  }
  if(!this->IntensityDistributionNotebook->IsCreated())
  {
    this->IntensityDistributionNotebook->SetParent(parent);
    this->IntensityDistributionNotebook->Create();
    this->IntensityDistributionNotebook->AddPage(
      "Intensity Distribution");
    //this->IntensityDistributionNotebook->AddPage("Manual S");
    //this->IntensityDistributionNotebook->AddPage("Labelmap");
    this->IntensityDistributionNotebook->AddPage("Visualization");
  }

  vtkKWFrame *intensity_page = this->IntensityDistributionNotebook->
    GetFrame("Intensity Distribution");
  //vtkKWFrame *manual_sampling_page =
    //this->IntensityDistributionNotebook->GetFrame("Manual S");
  //vtkKWFrame *labelPage =
    //this->IntensityDistributionNotebook->GetFrame("Labelmap");
  vtkKWFrame *gaussianPage =
    this->IntensityDistributionNotebook->GetFrame("Visualization");

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2",
    this->IntensityDistributionNotebook->GetWidgetName());

  // Create the distribution specification menu button

  /*if (!this->IntensityDistributionSpecificationMenuButton)
    {
    this->IntensityDistributionSpecificationMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }

  if (!this->IntensityDistributionSpecificationMenuButton->IsCreated())
    {
    this->IntensityDistributionSpecificationMenuButton->SetParent(
      intensity_page);
    this->IntensityDistributionSpecificationMenuButton->Create();
    this->IntensityDistributionSpecificationMenuButton->SetLabelText(
        "Specification:");
    this->IntensityDistributionSpecificationMenuButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH);
    this->IntensityDistributionSpecificationMenuButton->SetBalloonHelpString(
      "Select intensity distribution specification type.");
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
    this->IntensityDistributionSpecificationMenuButton->GetWidgetName());
*/
  
   // Create the Labelmap page

  // Create the preview volume selector
  if (!this->LabelSelector)
    {
    this->LabelSelector = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->LabelSelector->IsCreated())
    {
    this->LabelSelector->SetNodeClass(
      "vtkMRMLScalarVolumeNode",
      "LabelMap", "1",
      "LabelMap Sampling");
    this->LabelSelector->SetNewNodeEnabled(0);
    this->LabelSelector->SetParent(
      intensity_page);
    this->LabelSelector->Create();
    this->LabelSelector->
      SetMRMLScene(mrmlManager->GetMRMLScene());

    this->LabelSelector->SetBorderWidth(2);
    this->LabelSelector->SetLabelText( "Preview Labelmap: ");
    this->LabelSelector->SetBalloonHelpString(
      "select an preview labelmap from the current mrml scene.");
    }
  this->LabelSelector->UpdateMenu();
  if(mrmlManager->GetOutputVolumeMRMLID())
    {
    this->LabelSelector->SetSelected(
      this->LabelSelector->GetMRMLScene()->
      GetNodeByID(mrmlManager->GetOutputVolumeMRMLID()));
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2",
    this->LabelSelector->GetWidgetName());

    if (!this->LabelmapButton)
    {
    this->LabelmapButton = vtkKWPushButton::New();
    }

  this->AddLabelSelectorGUIObservers();

  // Button to compute mean and covariance from labelmap

  if (!this->LabelmapButton->IsCreated())
    {
    this->LabelmapButton->SetParent(intensity_page);
    this->LabelmapButton->Create();
    this->LabelmapButton->SetText("Compute class distribution");
    }

  this->Script(
      "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
      this->LabelmapButton->GetWidgetName());

  // Add observer for callbacks

  this->AddLabelButtonGUIEventsObservers();
  

  // Create the distribution mean vector/matrix

  if (!this->IntensityDistributionMeanMatrix)
    {
    this->IntensityDistributionMeanMatrix = vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->IntensityDistributionMeanMatrix->IsCreated())
    {
    this->IntensityDistributionMeanMatrix->SetParent(intensity_page);
    this->IntensityDistributionMeanMatrix->Create();
    this->IntensityDistributionMeanMatrix->SetLabelText("Log Mean:");
    this->IntensityDistributionMeanMatrix->ExpandWidgetOff();
    this->IntensityDistributionMeanMatrix->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH);
    this->IntensityDistributionMeanMatrix->SetBalloonHelpString(
        "Set the intensity distribution mean.");

    vtkKWMatrixWidget *matrix = this->IntensityDistributionMeanMatrix->
      GetWidget();
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
#if 0
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

    vtkKWMultiColumnList *list = this->IntensityDistributionManualSamplingList
      ->GetWidget()->GetWidget();
    list->SetRightClickCommand(this,
        "PopupManualIntensitySampleContextMenuCallback");
    list->SetHeight(4);
    list->MovableColumnsOff();
    list->SetSelectionModeToSingle();
    list->ResizableColumnsOff();
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -expand y -padx 0 -pady 2",
    this->IntensityDistributionManualSamplingList->GetWidgetName());
#endif
  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeIntensityDistributionsCallback();

 

  // Create the Gaussian 2D page

  std::string msg = "Select an image to define the range of the X axis in ";
  msg += "the 2D gaussian distribution.";
/*
  if (!this->Gaussian2DVolumeXMenuButton)
    {
    this->Gaussian2DVolumeXMenuButton = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->Gaussian2DVolumeXMenuButton->IsCreated())
    {
    this->Gaussian2DVolumeXMenuButton->SetParent(gaussianPage);
    this->Gaussian2DVolumeXMenuButton->Create();
    this->Gaussian2DVolumeXMenuButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH+10);
    this->Gaussian2DVolumeXMenuButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->Gaussian2DVolumeXMenuButton->SetLabelText("Image X Axis:");
    this->Gaussian2DVolumeXMenuButton->SetBalloonHelpString(msg.c_str());
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 5",
    this->Gaussian2DVolumeXMenuButton->GetWidgetName());
    */
  if (!this->Gaussian2DVolumeXMenuButton)
    {
    this->Gaussian2DVolumeXMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->Gaussian2DVolumeXMenuButton->IsCreated())
    {
    this->Gaussian2DVolumeXMenuButton->SetParent(gaussianPage);
    this->Gaussian2DVolumeXMenuButton->Create();
    this->Gaussian2DVolumeXMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->Gaussian2DVolumeXMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->Gaussian2DVolumeXMenuButton->
      SetLabelText("Image X Axis:");
    this->Gaussian2DVolumeXMenuButton->
      SetBalloonHelpString(
          "Select a target image to adjust intensity distribution");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
    this->Gaussian2DVolumeXMenuButton->GetWidgetName());

  this->PopulateGaussian2DVolumeXSelector();

  /*
  if(mrmlManager->GetTargetNumberOfSelectedVolumes() >0){
  this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->
  GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(0)));
  }
  */

 if (!this->Gaussian2DVolumeYMenuButton)
    {
    this->Gaussian2DVolumeYMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }

  if (!this->Gaussian2DVolumeYMenuButton->IsCreated())
    {
    this->Gaussian2DVolumeYMenuButton->SetParent(gaussianPage);
    this->Gaussian2DVolumeYMenuButton->Create();
    this->Gaussian2DVolumeYMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->Gaussian2DVolumeYMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->Gaussian2DVolumeYMenuButton->
      SetLabelText("Image Y Axis:");
    this->Gaussian2DVolumeYMenuButton->
      SetBalloonHelpString(
          "Select a target image to adjust intensity distribution");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
    this->Gaussian2DVolumeYMenuButton->GetWidgetName());

  this->PopulateGaussian2DVolumeYSelector();

  /*if(mrmlManager->GetTargetNumberOfSelectedVolumes()>0)
    {
    this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->
    GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(0)));
    if(mrmlManager->GetTargetNumberOfSelectedVolumes()>1)
      {
      this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->
      GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(1)));
      }
    }

  this->PopulateGaussian2DVolumeXSelector();

  vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
    this->Gaussian2DVolumeXMenuButton->SetEnabled(
    mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->Gaussian2DVolumeXMenuButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int volId = mrmlManager->GetVolumeNthID(0);
    this->Gaussian2DVolumeXSelectionChangedCallback(volId);
    }
*/
  this->NumberOfLeaves = 0;

  if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL, root_id);
      }
    }

  int windowWidth  = 300;
  int windowHeight = 300;

  // Add a render widget, attach it to the view frame, and pack

  if (!this->Gaussian2DWidget)
    {
    this->Gaussian2DWidget = vtkGaussian2DWidget::New();
    this->Gaussian2DWidget->SetParent(gaussianPage);
    this->Gaussian2DWidget->Create();
    this->Gaussian2DWidget->GetRenderer()->GetRenderWindow()->GetInteractor()
      ->Disable();

    this->Gaussian2DWidget->SetWidth(windowWidth);
    this->Gaussian2DWidget->SetHeight(windowHeight);
    this->Gaussian2DWidget->Render();
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2 -pady 2",
      this->Gaussian2DWidget->GetWidgetName());
      
 if (!this->Gaussian2DButton)
    {
    vtkErrorMacro("Gaussian2DButton New");
    this->Gaussian2DButton = vtkKWPushButton::New();
    }

  if (!this->Gaussian2DButton->IsCreated())
    {
    vtkErrorMacro("Gaussian2DButton->SetParent");
    this->Gaussian2DButton->SetParent(gaussianPage);
    this->Gaussian2DButton->Create();
    this->Gaussian2DButton->SetText("2D distributions");
    }

  this->Script(
      "pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
      this->Gaussian2DButton->GetWidgetName());

  this->AddGaussian2DButtonGUIEvents();

  // Create the histogram volume selector

  /*if (!this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton =
      vtkKWMenuButtonWithLabel::New();
    }

  if (!this->IntensityDistributionHistogramButton->IsCreated())
    {
    this->IntensityDistributionHistogramButton->SetParent(gaussianPage);
    this->IntensityDistributionHistogramButton->Create();
    this->IntensityDistributionHistogramButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->IntensityDistributionHistogramButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->IntensityDistributionHistogramButton->
      SetLabelText("Target Image:");
    this->IntensityDistributionHistogramButton->
      SetBalloonHelpString(
          "Select a target image to adjust intensity distribution");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
    this->IntensityDistributionHistogramButton->GetWidgetName());

  this->PopulateIntensityDistributionTargetVolumeSelector();

  vtkEMSegmentMRMLManager *mrmlManager0 = this->GetGUI()->GetMRMLManager();
  this->IntensityDistributionHistogramButton->SetEnabled(
      mrmlManager0->GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if(this->IntensityDistributionHistogramButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int volId = mrmlManager->GetVolumeNthID(0);
    this->IntensityDistributionTargetSelectionChangedCallback(volId);
    }*/

  // Create the Gaussian 2D rendering menu button

  /*if (!this->Gaussian2DRenderingMenuButton)
    {
    this->Gaussian2DRenderingMenuButton = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->Gaussian2DRenderingMenuButton->IsCreated())
    {
    this->Gaussian2DRenderingMenuButton->SetParent(gaussianPage);
    this->Gaussian2DRenderingMenuButton->Create();
    this->Gaussian2DRenderingMenuButton->SetLabelText("Rendering:");
    this->Gaussian2DRenderingMenuButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH);
    this->Gaussian2DRenderingMenuButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH);
    this->Gaussian2DRenderingMenuButton->SetBalloonHelpString(
        "Select rendering type.");
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2",
    this->Gaussian2DRenderingMenuButton->GetWidgetName());

  char buffer[1024];

  {
    vtkKWMenu *menu = this->Gaussian2DRenderingMenuButton->GetWidget()->
      GetMenu();
    menu->DeleteAllItems();

    bool validSelection = true;
    int selVolId = 1;

    if(validSelection)
    {
      vtksys_stl::string value;
      this->Gaussian2DRenderingMenuButton->SetEnabled(1);

      sprintf( buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationManual);
      menu->AddRadioButton("2D", this, buffer);

      sprintf( buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId),vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample);
      menu->AddRadioButton("3D", this, buffer);

      sprintf( buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample);
      menu->AddRadioButton("Auto Rendering", this, buffer);

      // temporarily disable auto sampling because it is not currently
      // implemented
      menu->SetItemStateToDisabled("Auto Rendering");

      switch (mrmlManager->GetTreeNodeDistributionSpecificationMethod(
            selVolId))
        {
        case vtkEMSegmentMRMLManager::DistributionSpecificationManual:
          value = "2D";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample:
          value = "3D";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample:
          value = "Auto Rendering";
          break;
        }
      //this->IntensityDistributionSpecificationMenuButton->GetWidget()->
      //  SetValue(value.c_str());
      }
    else
      {
      //this->IntensityDistributionSpecificationMenuButton->SetEnabled(0);
      //this->IntensityDistributionSpecificationMenuButton->GetWidget()->
      //  SetValue("");
      }
    }*/
  vtkEMSegmentIntensityDistributionsStep_DebugMacro("ShowUserInterface end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
PopulateGaussian2DVolumeXSelector()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateGaussian2DVolumeXSelector");

  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

  vtkKWMenu* menu = this->Gaussian2DVolumeXMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numTargets; i++)
  {
    target_vol_id = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d",
        "IntensityDistributionTargetSelectionChangedCallback",
        static_cast<int>(target_vol_id));
    const char *name = mrmlManager->GetVolumeName(target_vol_id);
    if (name)
    {
      menu->AddRadioButton(name, this, buffer);
      if(i == 0)
      {
        this->Gaussian2DVolumeXMenuButton->GetWidget()->SetValue(name);
      }
    }
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateGaussian2DVolumeXSelector end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  PopulateGaussian2DVolumeYSelector()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateGaussian2DVolumeYSelector");

  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if(!mrmlManager)
    {
    return;
    }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

  vtkKWMenu* menu = this->Gaussian2DVolumeYMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numTargets; i++)
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
        this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(name);
        }
      }
    }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateGaussian2DVolumeYSelector end");
}

//----------------------------------------------------------------------------
/*void vtkEMSegmentIntensityDistributionsStep::
PopulateIntensityDistributionTargetVolumeSelector()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateIntensityDistributionTargetVolumeSelector");

  vtkIdType target_volId;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

  vtkKWMenu* menu = this->IntensityDistributionHistogramButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numTargets; i++)
    {
    target_volId = mrmlManager->GetTargetSelectedVolumeNthID(i);
    sprintf(buffer, "%s %d",
            "IntensityDistributionTargetSelectionChangedCallback",
            static_cast<int>(target_volId));
    const char *name = mrmlManager->GetVolumeName(target_volId);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopulateIntensityDistributionTargetVolumeSelector end");
}
*/
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  Gaussian2DVolumeXSelectionChangedCallback(vtkIdType vtkNotUsed(target_volId))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "Gaussian2DVolumeXSelectionChangedCallback");

  //std::cout << __LINE__ << " vol id: "<< target_volId <<std::endl;

  //vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(
  //    target_volId)->GetImageData()->GetPointData()->GetScalars();

  //double* range = array->GetRange();
}

//----------------------------------------------------------------------------
/*void vtkEMSegmentIntensityDistributionsStep::
PopulateGaussian2DVolumeXSelector()
{
  std::cout << __LINE__ << " PopulateGaussian2DVolumeXSelector" << std::endl;
  vtkIdType target_volId;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();
  vtkKWMenu* menu = this->Gaussian2DVolumeXMenuButton->GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numTargets; i++)
    {
    target_volId = mrmlManager->GetTargetSelectedVolumeNthID(i);

    sprintf(buffer, "%s %d", "Gaussian2DVolumeXSelectionChangedCallback",
        static_cast<int>(target_volId));

    const char *name = mrmlManager->GetVolumeName(target_volId);

    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }

  std::cout << __LINE__ << " PopulateGaussian2DVolumeXSelector end"
    << std::endl;
}*/

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::HideUserInterface()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro("HideUserInterface");

  this->Superclass::HideUserInterface();
  this->RemoveManualIntensitySamplingGUIObservers();
  this->RemoveGaussian2DButtonGUIEvents();
  this->RemoveLabelButtonGUIEventsObservers();
  this->RemoveLabelSelectorGUIObservers();

  vtkEMSegmentIntensityDistributionsStep_DebugMacro("HideUserInterface end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
DisplaySelectedNodeIntensityDistributionsCallback()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DisplaySelectedNodeIntensityDistributionsCallback");

  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  vtkEMSegmentAnatomicalStructureStep *anatomicalStep =
    this->GetGUI()->GetAnatomicalStructureStep();

  if (!anatomicalStep)
  {
    return;
  }

  vtkKWTree *tree = anatomicalStep->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType selVolId = 0;

  int manually_sample_mode = 0;
  int label_sample_mode = 0;
  int hasValidSelection = tree->HasSelection();

  if (hasValidSelection)
  {
    sel_node = tree->GetSelection();
    selVolId = tree->GetNodeUserDataAsInt(sel_node.c_str());
    hasValidSelection = mrmlManager->GetTreeNodeIsLeaf(selVolId);
    manually_sample_mode =
      mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) ==
      vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample;
    label_sample_mode =
      mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) ==
      vtkEMSegmentMRMLManager::DistributionSpecificationLabelSample;
  }

  int enabled = tree->GetEnabled();
  int row, col;
  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];

  // Update the distribution specification menu button

  /*if (this->IntensityDistributionSpecificationMenuButton)
    {
    vtkKWMenu *menu = this->IntensityDistributionSpecificationMenuButton->
      GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (hasValidSelection)
      {
      vtksys_stl::string value;
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(enabled);*/
      //sprintf(
      //  buffer, "IntensityDistributionSpecificationCallback %d %d",
      //  static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
      //  DistributionSpecificationManual);
      //menu->AddRadioButton("Manual", this, buffer);
      //sprintf(
      //  buffer, "IntensityDistributionSpecificationCallback %d %d",
      //  static_cast<int>(selVolId),vtkEMSegmentMRMLManager::
      //  DistributionSpecificationManuallySample);
      //menu->AddRadioButton("Manual Sampling", this, buffer);
      /*sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationLabelSample);
      menu->AddRadioButton("Labelmap Sampling", this, buffer);*/
      //sprintf(
      //  buffer, "IntensityDistributionSpecificationCallback %d %d",
      //  static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
      //  DistributionSpecificationAutoSample);
      //menu->AddRadioButton("Auto Sampling", this, buffer);

      // temporarily disable auto sampling because it is not currently
      // implemented
      //menu->SetItemStateToDisabled("Auto Sampling");
/*
      switch(mrmlManager->GetTreeNodeDistributionSpecificationMethod(
            selVolId))
        {*/
        //case vtkEMSegmentMRMLManager::DistributionSpecificationManual:
        //  value = "Manual";
        //  break;
        //case vtkEMSegmentMRMLManager::
        //DistributionSpecificationManuallySample:
        //  value = "Manual Sampling";
        //  break;
       /* case vtkEMSegmentMRMLManager::
        DistributionSpecificationLabelSample:
          value = "Label Sampling";
          break;*/
        //case vtkEMSegmentMRMLManager::
        //DistributionSpecificationAutoSample:
        //  value = "Auto Sampling";
        //  break;
     /*   }
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
*/
  // Update the distribution mean vector/matrix

  if (this->IntensityDistributionMeanMatrix)
    {
    vtkKWMatrixWidget *matrix =
      this->IntensityDistributionMeanMatrix->GetWidget();
    if (hasValidSelection)
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(
        numTargets ? enabled : 0);
      matrix->SetNumberOfColumns(numTargets);
      matrix->SetNumberOfRows(1);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer, "IntensityDistributionMeanChangedCallback %d", 
        static_cast<int>(selVolId));
      matrix->SetElementChangedCommand(this, buffer);

      for(col = 0; col < numTargets; col++)
        {
        matrix->SetElementValueAsDouble(
          0, col, 
          mrmlManager->GetTreeNodeDistributionLogMean(selVolId, col));
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
    if (hasValidSelection)
      {
      this->IntensityDistributionCovarianceMatrix->SetEnabled(
        numTargets ? enabled : 0);
      matrix->SetNumberOfColumns(numTargets);
      matrix->SetNumberOfRows(numTargets);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer,"IntensityDistributionCovarianceChangedCallback %d",
        static_cast<int>(selVolId));
      matrix->SetElementChangedCommand(this, buffer);

      for (row = 0; row < numTargets; row++)
        {
        for (col = 0; col < numTargets; col++)
          {
          matrix->SetElementValueAsDouble(
            row, col, 
            mrmlManager->GetTreeNodeDistributionLogCovariance(
              selVolId, row, col));
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
      "Manual S", manually_sample_mode);
    }

  if (this->IntensityDistributionManualSamplingList)
    {
    vtkKWMultiColumnList *list = this->IntensityDistributionManualSamplingList
      ->GetWidget()->GetWidget();
    list->DeleteAllRows();
    if (hasValidSelection && manually_sample_mode)
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(enabled);
      int nb_cols = list->GetNumberOfColumns();
      for (;nb_cols < numTargets; nb_cols++)
        {
        int col_id = list->AddColumn("");
        list->SetColumnWidth(col_id, 0);
        list->ColumnStretchableOff(col_id);
        list->SetColumnSortMode(col_id, vtkKWMultiColumnList::SortModeReal);
        }
      for (;nb_cols > numTargets; nb_cols--)
        {
        list->DeleteColumn(nb_cols - 1);
        }
      for (col = 0; col < numTargets; ++col)
        {
        vtkIdType volumeID = mrmlManager->GetTargetSelectedVolumeNthID(col);
        const char* title = mrmlManager->GetVolumeName(volumeID);
        list->SetColumnTitle(col, title);
        }
      double intensity;
      int nb_samples =
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(selVolId);
      for (row = 0; row < nb_samples; row++)
        {
        list->AddRow();
        for (col = 0; col < numTargets; col++)
          {
          int volId = mrmlManager->GetTargetSelectedVolumeNthID(col);
          intensity = mrmlManager->
            GetTreeNodeDistributionSampleIntensityValue(
            selVolId, row, volId);
          list->SetCellTextAsDouble(row, col, intensity);
          }
        }
      }
    else
      {
      this->IntensityDistributionManualSamplingList->SetEnabled(0);
      }
    }

    // Enable/disable labelmap page

    if (this->IntensityDistributionNotebook)
    {
      this->IntensityDistributionNotebook->SetPageEnabled(
          "Labelmap S", label_sample_mode);
    }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DisplaySelectedNodeIntensityDistributionsCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionSpecificationCallback(
  vtkIdType selVolId, int type)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionSpecificationCallback");

  // The distribution specification has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  if (type != mrmlManager->
      GetTreeNodeDistributionSpecificationMethod(selVolId))
  {
    mrmlManager->SetTreeNodeDistributionSpecificationMethod(selVolId, type);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionSpecificationCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::Gaussian2DRenderingCallback(
  vtkIdType vtkNotUsed(selVolId), int vtkNotUsed(type))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "Gaussian2DRenderingCallback");

  // The rendering type has changed because of user interaction
  /*
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  if (type != mrmlManager->
      GetTreeNodeDistributionSpecificationMethod(selVolId))
    {
    mrmlManager->SetTreeNodeDistributionSpecificationMethod(selVolId, type);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
    }
    */
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionMeanChangedCallback(
    vtkIdType selVolId, int vtkNotUsed(row), int col, const char *value)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionMeanChangedCallback");

  // The distribution mean vector has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  mrmlManager->SetTreeNodeDistributionLogMean(selVolId, col, atof(value));

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionMeanChangedCallback end");
}

//-------------------------------------------------------------------
void
vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionCovarianceChangedCallback(
  vtkIdType selVolId, int row, int col, const char *value)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionCovarianceChangedCallback");

  // The distribution covariance matrix has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  mrmlManager->SetTreeNodeDistributionLogCovariance(selVolId, row,
      col, atof(value));

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionCovarianceChangedCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
AddIntensityDistributionSamplePoint(double ras[3])
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddIntensityDistributionSamplePoint");

  // Since it is not a callback, make sure we are really allowed to add
  // a sample point now

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();

  if (!wizardWidget)
  {
    return;
  }

  if(wizardWidget->GetWizardWorkflow()->GetCurrentStep() != this)
  {
    return;
  }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  vtkEMSegmentAnatomicalStructureStep *anatomicalStep =
    this->GetGUI()->GetAnatomicalStructureStep();

  if (!anatomicalStep)
  {
    return;
  }

  vtkKWTree *tree = anatomicalStep->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType selVolId;

  if (tree->HasSelection())
    {
    sel_node = tree->GetSelection();
    selVolId = tree->GetNodeUserDataAsInt(sel_node.c_str());
    if (sel_node.size() &&
        mrmlManager->GetTreeNodeIsLeaf(selVolId) &&
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(
          selVolId) ==
        vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample)
      {
      mrmlManager->AddTreeNodeDistributionSamplePoint(selVolId, ras);
      this->DisplaySelectedNodeIntensityDistributionsCallback();
      int nb_samples =
        mrmlManager->GetTreeNodeDistributionNumberOfSamples(selVolId);
      vtkKWMultiColumnList *list =
       this->IntensityDistributionManualSamplingList->GetWidget()->
       GetWidget();
      list->SeeRow(nb_samples - 1);
      }
    }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddIntensityDistributionSamplePoint end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
PopupManualIntensitySampleContextMenuCallback(int row, int, int x, int y)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopupManualIntensitySampleContextMenuCallback");

  vtkEMSegmentAnatomicalStructureStep *anatomicalStep =
    this->GetGUI()->GetAnatomicalStructureStep();

  vtkKWTree *tree = anatomicalStep->GetAnatomicalStructureTree()->GetWidget();

  if (!tree->HasSelection())
  {
    return;
  }

  vtksys_stl::string sel_node(tree->GetSelection());
  vtkIdType selVolId = tree->GetNodeUserDataAsInt(sel_node.c_str());

  vtkKWMultiColumnList *list = this->
    IntensityDistributionManualSamplingList->GetWidget()->GetWidget();
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

  if(row >= 0)
  {
    sprintf( buffer, "DeleteManualIntensitySampleCallback %d %d",
      static_cast<int>(selVolId), row);
    this->ContextMenu->AddCommand("Delete sample", this, buffer);
  }

  if(list->GetNumberOfRows())
  {
    sprintf(buffer, "DeleteAllManualIntensitySampleCallback %d",
            static_cast<int>(selVolId));
    this->ContextMenu->AddCommand("Delete all samples", this, buffer);
  }

  this->ContextMenu->PopUp(x,y);

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "PopupManualIntensitySampleContextMenuCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
DeleteManualIntensitySampleCallback( vtkIdType selVolId,
    int sample_index)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DeleteManualIntensitySampleCallback");

  // A sample has been deleted because of user interaction

  if(sample_index >= 0)
  {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->
      GetMRMLManager();

    if (!mrmlManager)
    {
      return;
    }

    vtkKWMultiColumnList *list =
      this->IntensityDistributionManualSamplingList->GetWidget()->
      GetWidget();

    if (!list)
    {
      return;
    }

    list->DeleteRow(sample_index);
    mrmlManager->RemoveTreeNodeDistributionSamplePoint(selVolId,
        sample_index);
    this->DisplaySelectedNodeIntensityDistributionsCallback();
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DeleteManualIntensitySampleCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
DeleteAllManualIntensitySampleCallback(vtkIdType selVolId)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DeleteAllManualIntensitySampleCallback");

  // All samples have been deleted because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  vtkKWMultiColumnList *list = this->
    IntensityDistributionManualSamplingList->GetWidget()->GetWidget();

  if (!mrmlManager || !list)
  {
    return;
  }

  list->DeleteAllRows();
  mrmlManager->RemoveAllTreeNodeDistributionSamplePoints(selVolId);
  this->DisplaySelectedNodeIntensityDistributionsCallback();

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "DeleteAllManualIntensitySampleCallback end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
AddManualIntensitySamplingGUIObservers()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddManualIntensitySamplingGUIObservers");

  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent,
    this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent,
    this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(
    vtkCommand::LeftButtonPressEvent,this->GetGUI()->GetGUICallbackCommand());

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddManualIntensitySamplingGUIObservers end");
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
RemoveManualIntensitySamplingGUIObservers()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "RemoveManualIntensitySamplingGUIObservers");

  // Slice GUI 0

  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent,this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent,this->GetGUI()->GetGUICallbackCommand());

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->RemoveObservers(
    vtkCommand::LeftButtonPressEvent,this->GetGUI()->GetGUICallbackCommand());

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "RemoveManualIntensitySamplingGUIObservers end");
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
ProcessManualIntensitySamplingGUIEvents(
  vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "ProcessManualIntensitySamplingGUIEvents");

  vtkSlicerInteractorStyle *s =
    vtkSlicerInteractorStyle::SafeDownCast(caller);

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

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "ProcessManualIntensitySamplingGUIEvents end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::PrintSelf(ostream& os, vtkIndent
    indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetNumberOfLeaf(
  const char *vtkNotUsed(parent), vtkIdType volId)
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "GetNumberOfLeaf");

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int numChildren = mrmlManager->GetTreeNodeNumberOfChildren(volId);

  if(numChildren == 0)
  {
    this->NumberOfLeaves = this->NumberOfLeaves +1;
    this->LeafId[this->NumberOfLeaves-1] = static_cast<int>(volId);
    this->LeafLabel[this->NumberOfLeaves-1] = mrmlManager->
      GetTreeNodeIntensityLabel(this->LeafId[this->NumberOfLeaves-1]);
  }

  for (int i = 0; i < numChildren; i++)
  {
    this->GetNumberOfLeaf( NULL, mrmlManager->GetTreeNodeChildNodeID(volId,
          i));
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "GetNumberOfLeaf end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionTargetSelectionChangedCallback(vtkIdType vtkNotUsed(targetVolId))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionTargetSelectionChangedCallback");

  double meanX;
  double meanY;
  double varianceX;
  double varianceY;
  double covariance;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  this->GetGaussian2DTargetVolumes();

  vtkMRMLScene     *mrmlScene = mrmlManager->GetMRMLScene();
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(mrmlScene->
      GetNodeByID( mrmlManager->GetColormap() ));

  double hsv[3];

  this->Gaussian2DWidget->RemoveAllGaussians();

  this->Gaussian2DWidget->SetScalarRangeX(0,1);
  this->Gaussian2DWidget->SetScalarRangeY(0,1);

  for(int m=0; m < this->NumberOfLeaves; m++)
  {
    vtkIdType leaf  = this->LeafId[m];
    vtkIdType label = mrmlManager->GetTreeNodeIntensityLabel(leaf);

    double *color = colorNode->GetLookupTable()->GetTableValue(label);

    vtkMath::RGBToHSV(color,hsv);

    vtkIdType i = this->TargetVolumeX;
    vtkIdType j = this->TargetVolumeY;

    meanX = mrmlManager->GetTreeNodeDistributionMean(leaf,i);
    meanY = mrmlManager->GetTreeNodeDistributionMean(leaf,j);

    varianceX = mrmlManager->GetTreeNodeDistributionCovariance(leaf,i,i);
    varianceY = mrmlManager->GetTreeNodeDistributionCovariance(leaf,j,j);

    covariance = mrmlManager->GetTreeNodeDistributionCovariance(leaf,i,j);

    this->Gaussian2DWidget->AddGaussian(
        meanX,
        meanY,
        varianceX,
        varianceY,
        covariance,
        hsv[0]);
  }

  this->Gaussian2DWidget->Render();
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "IntensityDistributionTargetSelectionChangedCallback end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessGaussian2DButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "ProcessGaussian2DButtonGUIEvents");

  if (event == vtkKWPushButton::InvokedEvent && caller ==
      this->Gaussian2DButton)
  {
  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddGaussian2DButtonGUIEvents()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddGaussian2DButtonGUIEvents");

  this->Gaussian2DButton->AddObserver(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveGaussian2DButtonGUIEvents()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "RemoveGaussian2DButtonGUIEvents");

  this->Gaussian2DButton->RemoveObservers(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetGaussian2DTargetVolumes()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "GetGaussian2DTargetVolumes");

  vtkIdType target_volId;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

  for(int i = 0; i < numTargets; i++)
  {
    target_volId = mrmlManager->GetTargetSelectedVolumeNthID(i);
    const char *name = mrmlManager->GetVolumeName(target_volId);

    vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(
      target_volId)->GetImageData()->GetPointData()->GetScalars();

    double* range = array->GetRange();

    if(!strcmp(this->Gaussian2DVolumeXMenuButton->GetWidget()->GetValue(),
          name))
    {
      this->TargetVolumeX=i;
      this->TargetVolumeXRange=range;
    }

    if(!strcmp(this->Gaussian2DVolumeYMenuButton->GetWidget()->GetValue(),
          name))
    {
      this->TargetVolumeY=i;
      this->TargetVolumeYRange=range;
    }
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "GetGaussian2DTargetVolumes end");
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
AddLabelButtonGUIEventsObservers()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "AddLabelButtonGUIEventsObservers");

  this->LabelmapButton->AddObserver(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
RemoveLabelButtonGUIEventsObservers()
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "RemoveLabelButtonGUIEventsObservers");

  this->LabelmapButton->RemoveObservers(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessLabelButtonGUIEvents(
  vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "ProcessLabelButtonGUIEvents");

  if(event == vtkKWPushButton::InvokedEvent &&
    caller == this->LabelmapButton)
  {
    vtkEMSegmentMRMLManager *mrmlManager =
      this->GetGUI()->GetMRMLManager();

    vtkMRMLScalarVolumeNode *labelVolume =
      mrmlManager->GetOutputVolumeNode();

    if(labelVolume == NULL)
    {
      vtkErrorMacro("No label volume found");
      return;
    }

    if(labelVolume->GetImageData() == NULL)
    {
      vtkErrorMacro("Label Map has no data");
      return;
    }

    if(labelVolume->GetImageData()->GetScalarType() != VTK_SHORT)
    {
      vtkErrorMacro("Scalar type of Label Map must be SHORT. Label Map type: "
      << labelVolume->GetImageData()->GetScalarTypeAsString());
      return;
    }

    if (labelVolume->GetImageData()->GetNumberOfScalarComponents() != 1)
    {
      vtkErrorMacro("Label map must have voxels with 1 component. "
      << "The voxels in the label map have "
      << labelVolume->GetImageData()->GetNumberOfScalarComponents()
      << " components.");
      return;
    }

    int dim[3];
    int labelExtent[6];

    labelVolume->GetImageData()->GetDimensions(dim);
    labelVolume->GetImageData()->GetWholeExtent(labelExtent);

    if (dim[0] == 0 && dim[1] == 0 && dim[2] == 0)
    {
      vtkErrorMacro("Invalid dimensions for LabelMap: "
        << dim[0] << " " << dim[1] << " " << dim[2]);
      return;
    }

    if (labelExtent[1] < labelExtent[0] ||
        labelExtent[3] < labelExtent[2] ||
        labelExtent[5] < labelExtent[4])
    {
      vtkErrorMacro("Invalid extent for LabelMap:"
        << " " << labelExtent[0] << " " << labelExtent[1]
        << " " << labelExtent[2] << " " << labelExtent[3]
        << " " << labelExtent[4] << " " << labelExtent[5]);
      return;
    }

    int numTargetImages = mrmlManager->GetTargetNumberOfSelectedVolumes();

    if (numTargetImages < 1)
    {
      vtkErrorMacro("Invalid number of target images: " << numTargetImages);
      return;
    }

    vtkIdType volumeID;
    int extent[6];
    vtkstd::vector<double>       totalMinRange(numTargetImages, 0);
    double* rangeMin;
    //double rangeMax;


    for (int m=0; m < numTargetImages; m++)
    {
      volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);

      vtkMRMLVolumeNode *targetVolume = mrmlManager->GetVolumeNode(volumeID);

      if (targetVolume == NULL)
      {
        vtkErrorMacro("No target volume for ID " << volumeID);
        return;
      }

      vtkImageData *targetImage = targetVolume->GetImageData();

      if (targetImage == NULL)
      {
        vtkErrorMacro("Target volume has no data");
        return;
      }

      targetImage->GetWholeExtent(extent);

      if (extent[0] != labelExtent[0] || extent[1] != labelExtent[1] ||
          extent[2] != labelExtent[2] || extent[3] != labelExtent[3] ||
          extent[4] != labelExtent[4] || extent[5] != labelExtent[5])
      {
        vtkErrorMacro("Label Map has whole extent "
          << labelExtent[0] << " " << labelExtent[1] << " "
          << labelExtent[2] << " " << labelExtent[3] << " "
          << labelExtent[4] << " " << labelExtent[5]
          << " while Target Volume " << volumeID << " has whole extent "
          << extent[0] << " " << extent[1] << " "
          << extent[2] << " " << extent[3] << " "
          << extent[4] << " " << extent[5]);
        return;
      }

      if (targetImage->GetNumberOfScalarComponents() != 1)
      {
        vtkErrorMacro("Voxels in Target Volume must have 1 component. "
          << "The voxels in the Target Volume " << volumeID << " have "
          << targetImage->GetNumberOfScalarComponents()
          << " components.");
        return;
      }
      
      rangeMin = targetImage->GetPointData()->GetScalars()->GetRange();
      
      totalMinRange[m] = rangeMin[0];
      
      std::cout<< "min: "<< totalMinRange[m] << std::endl;
    }

    int numValues = this->NumberOfLeaves * numTargetImages;

    vtkstd::vector<double>       meanIntensity(numValues, 0);
    vtkstd::vector<double>       meanLogIntensity(numValues, 0);
    vtkstd::vector<unsigned int> countIntensity(numValues, 0);
    vtkstd::vector<double>       minMaxRange(numTargetImages*2, 0);

    short *ptr =
      static_cast<short*>(labelVolume->GetImageData()->GetScalarPointer());

    int id;
    //double range[2] = {VTK_DOUBLE_MAX,VTK_DOUBLE_MIN};

    double intensity;
    double logIntensity;

    vtkMRMLVolumeNode *targetVolume;
    vtkImageData      *targetImage;

    for(int k=0; k < dim[2]; k++)
    {
      for(int j=0; j < dim[1]; j++)
      {
        for(int i=0; i < dim[0]; i++, ptr++)
        {
          short label = *ptr;

          if(label)
          {
            for(int l=0; l < this->NumberOfLeaves; l++)
            {
              if (label == this->LeafLabel[l])
              {
                for(int m=0; m < numTargetImages; m++)
                {
                  id = l * numTargetImages + m;

                  volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);

                  targetVolume = mrmlManager->GetVolumeNode(volumeID);

                  targetImage = targetVolume->GetImageData();

                  intensity =
                    targetImage->GetScalarComponentAsDouble(i,j,k,0);

                  meanIntensity[id]    += intensity;
                  meanLogIntensity[id] += log(intensity+1-totalMinRange[m]);

                  countIntensity[id]++;

                  if( intensity < minMaxRange[2*m] ) { minMaxRange[2*m] = intensity; }
                  if( intensity > minMaxRange[2*m+1] ) { minMaxRange[2*m+1] = intensity; }
                }
              }
            }
          }
        }
      }
    }

    /*if(range[0] >= range[1])
    {
      vtkErrorMacro("Invalid range " << range[0] << " " << range[1]);
      return;
    }*/
    
    double factor = 0.0;

    //double factor = 1.0 / (range[1]-range[0]);

    for(int j=0; j < this->NumberOfLeaves; j++)
    {
      for (int i=0; i < numTargetImages; i++)
      {
        id = j * numTargetImages + i;

        if (countIntensity[id])
        {
          meanIntensity[id]    /= static_cast<double>(countIntensity[id]);
          meanLogIntensity[id] /= static_cast<double>(countIntensity[id]);

          vtkErrorMacro("mean: "<<meanIntensity[id]);
          vtkErrorMacro("min: "<<minMaxRange[2*i]);
          vtkErrorMacro("max: "<<minMaxRange[2*i+1]);

          meanIntensity[id] -= minMaxRange[2*i];
          
          if(minMaxRange[2*i] >= minMaxRange[2*i+1])
            {
            vtkErrorMacro("Invalid range " << minMaxRange[2*i] << " " << minMaxRange[2*i+1]);
            return;
          }
          
          factor = 1.0 / (minMaxRange[2*i+1]-minMaxRange[2*i]);
          
          meanIntensity[id] *= factor;

          vtkErrorMacro("mean normalized: "<<meanIntensity[id]);
        }
      }
    }

    ptr =
      static_cast<short*>(labelVolume->GetImageData()->GetScalarPointer());

    numValues *= numTargetImages;

    vtkstd::vector<double>    covarianceMatrix(numValues,0.0);
    vtkstd::vector<double> covarianceLogMatrix(numValues,0.0);

    int id1;
    int id2;

    for(int k=0; k < dim[2]; k++)
    {
      for(int j=0; j < dim[1]; j++)
      {
        for(int i=0; i < dim[0]; i++, ptr++)
        {
          short label = *ptr;

          if (label)
          {
            for(int l=0; l < this->NumberOfLeaves; l++)
            {
              if(label == this->LeafLabel[l])
              {
                for(int m=0; m < numTargetImages; m++)
                {
                  id1 = l * numTargetImages + m;

                  volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);

                  targetVolume = mrmlManager->GetVolumeNode(volumeID);
                  targetImage = targetVolume->GetImageData();

                  double intensity1 =
                    targetImage->GetScalarComponentAsDouble(i,j,k,0);

                  double logIntensity1 = log(intensity1+1-totalMinRange[m]);
                  
                  if(minMaxRange[2*m] >= minMaxRange[2*m+1])
                    {
                    vtkErrorMacro("Invalid range " << minMaxRange[2*m] << " " << minMaxRange[2*m+1]);
                    return;
                    }
          
                  factor = 1.0 / (minMaxRange[2*m+1]-minMaxRange[2*m]);
                  
                  intensity1 -= minMaxRange[2*m];
                  intensity1 *= factor;

                  double diff1    = intensity1    - meanIntensity[id1];
                  double logDiff1 = logIntensity1 - meanLogIntensity[id1];

                  for(int n=0; n < numTargetImages; n++)
                  {
                    id2 = l * numTargetImages + n;

                    volumeID = mrmlManager->GetTargetSelectedVolumeNthID(n);

                    targetVolume = mrmlManager->GetVolumeNode(volumeID);
                    targetImage = targetVolume->GetImageData();

                    double intensity2 =
                      targetImage->GetScalarComponentAsDouble(i,j,k,0);

                    double logIntensity2 = log(intensity2+1-totalMinRange[n]);
                    
                    if(minMaxRange[2*n] >= minMaxRange[2*n+1])
                    {
                    vtkErrorMacro("Invalid range " << minMaxRange[2*n] << " " << minMaxRange[2*n+1]);
                    return;
                    }
          
                    factor = 1.0 / (minMaxRange[2*n+1]-minMaxRange[2*n]);
                    
                    intensity2 -= minMaxRange[2*n];
                    intensity2 *= factor;

                    double diff2    = intensity2    - meanIntensity[id2];
                    double logDiff2 = logIntensity2 - meanLogIntensity[id2];

                    id = id1 * numTargetImages + n;

                    covarianceMatrix[id]    += diff1 * diff2;
                    covarianceLogMatrix[id] += logDiff1 * logDiff2;
                  }
                }
                continue;
              }
            }
          }
        }
      }
    }

    for (int l=0; l < this->NumberOfLeaves; l++)
    {
      vtkIdType leaf = this->LeafId[l];

      for (int m=0; m < numTargetImages; m++)
      {
        id1 = l * numTargetImages + m;

        intensity    = meanIntensity[id1];
        logIntensity = meanLogIntensity[id1];

        mrmlManager->SetTreeNodeDistributionMean(leaf,m,intensity);
        mrmlManager->SetTreeNodeDistributionLogMean(leaf,m,logIntensity);

        factor = countIntensity[id1] > 1 ?
          1.0/static_cast<double>(countIntensity[id1]-1) : 1.0;

        for (int n=0; n < numTargetImages; n++)
        {
          id = id1 * numTargetImages + n;

          if (countIntensity[id1] > 1)
          {
            covarianceMatrix[id]    *= factor;
            covarianceLogMatrix[id] *= factor;
          }

          mrmlManager->SetTreeNodeDistributionLogCovariance(leaf,m,n,
            covarianceLogMatrix[id]);
          mrmlManager->SetTreeNodeDistributionCovariance(leaf,m,n,
            covarianceMatrix[id]);
        }
      }
    }
  }

  vtkEMSegmentIntensityDistributionsStep_DebugMacro(
      "ProcessLabelButtonGUIEvents end");
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddLabelSelectorGUIObservers()
{
  this->LabelSelector->AddObserver(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
    this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveLabelSelectorGUIObservers()
{
  this->LabelSelector->RemoveObservers(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
    this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessLabelSelectorGUIEvents(
  vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  if (caller == this->LabelSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
      this->LabelSelector->GetSelected() != NULL)
  {
    vtkEMSegmentMRMLManager *mrmlManager =
      this->GetGUI()->GetMRMLManager();

    if (mrmlManager)
    {
      mrmlManager->SetOutputVolumeMRMLID(
        this->LabelSelector->GetSelected()->GetID());
    }

    // find output volume
    if (!mrmlManager->GetSegmenterNode())
    {
      vtkErrorMacro("Segmenter Node is null: aborting segmentation.");
      return;
    }

    vtkMRMLScalarVolumeNode *outVolume =
      mrmlManager->GetOutputVolumeNode();

    if (outVolume == NULL)
    {
      vtkErrorMacro("No Output Volume: aborting segmentation.");
      return;
    }
  }
}

