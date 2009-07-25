/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkEMSegmentIntensityDistributionsStep.cxx,v$
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/
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

#include "vtkKWFrameWithLabel.h"
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
#include "vtkSlicerInteractorStyle.h"

#include "vtkMatrix4x4.h"
#include "vtkImageReslice.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkVolume.h"

#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"

#include "vtkImageInteractionCallback.h"

#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkImageData.h"
#include "vtkDataSetMapper.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

#include "vtkImageDataGeometryFilter.h"
#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkFloatArray.h"

#include "vtkGaussian2DWidget.h"

#include "vtkImageViewer2.h"
#include "vtkAxisActor2D.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkKWApplication.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWWindow.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkRenderWindow.h"
#include "vtkKWSimpleAnimationWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"

#define EMS_DEBUG_MACRO(msg) std::cout << __LINE__ << " EMSegment " << msg \
  << std::endl;

//-------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityDistributionsStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityDistributionsStep,
  "$Revision: 1.2 $");

//-------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::
vtkEMSegmentIntensityDistributionsStep()
{
  this->SetName("6/9. Specify Intensity Distributions");
  this->SetDescription(
    "Define intensity distribution for each anatomical structure.");

  this->IntensityDistributionNotebook                        = NULL;
  this->IntensityDistributionSpecificationMenuButton         = NULL;
  this->IntensityDistributionMeanMatrix                      = NULL;
  this->IntensityDistributionCovarianceMatrix                = NULL;
  this->IntensityDistributionManualSamplingList              = NULL;
  this->ContextMenu                                          = NULL;

  this->IntensityDistributionHistogramFrame                  = NULL;
  this->IntensityDistributionHistogramButton                 = NULL;

  this->Gaussian2DWidget                                     = NULL;
  this->NumClassesEntryLabel                                 = NULL;
  this->Gaussian2DButton                                     = NULL;
  this->Gaussian2DRenderingMenuButton                        = NULL;
  
  this->Gaussian2DVolumeXMenuButton                          = NULL;
  this->Gaussian2DVolumeYMenuButton                          = NULL;
  
  this->LabelSelector                                        = NULL;
  this->LabelmapButton                                       = NULL;

}

//-------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::
~vtkEMSegmentIntensityDistributionsStep()
{
  EMS_DEBUG_MACRO("destructor start");
  
  if (this->LabelSelector)
    {
    this->LabelSelector->Delete();
    this->LabelSelector = NULL;
    }
  
  if (this->Gaussian2DVolumeYMenuButton)
    {
    this->Gaussian2DVolumeYMenuButton->Delete();
    this->Gaussian2DVolumeYMenuButton = NULL;
    }
  
  if (this->Gaussian2DVolumeXMenuButton)
    {
    this->Gaussian2DVolumeXMenuButton->Delete();
    this->Gaussian2DVolumeXMenuButton = NULL;
    }
    
  if (this->LabelmapButton)
    {
    this->LabelmapButton->Delete();
    this->LabelmapButton = NULL;
    }
    
  if (this->Gaussian2DButton)
    {
    this->Gaussian2DButton->Delete();
    this->Gaussian2DButton = NULL;
    }

  if (this->NumClassesEntryLabel)
    {
    this->NumClassesEntryLabel->Delete();
    this->NumClassesEntryLabel = NULL;
    }

  if (this->IntensityDistributionHistogramButton)
    {
    this->IntensityDistributionHistogramButton->Delete();
    this->IntensityDistributionHistogramButton = NULL;
    }

  if (this->IntensityDistributionHistogramFrame)
    {
    this->IntensityDistributionHistogramFrame->Delete();
    this->IntensityDistributionHistogramFrame = NULL;
    }

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

  if (this->Gaussian2DWidget)
    {
    this->Gaussian2DWidget->Delete();
    this->Gaussian2DWidget = NULL;
    }

  if (this->Gaussian2DRenderingMenuButton)
    {
    this->Gaussian2DRenderingMenuButton->Delete();
    this->Gaussian2DRenderingMenuButton = NULL;
    }

  EMS_DEBUG_MACRO("destructor end");
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ShowUserInterface()
{
  EMS_DEBUG_MACRO("ShowUserInterface start");
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

  if (rootNode && *rootNode)
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
    this->IntensityDistributionNotebook->AddPage("Manual S");
    this->IntensityDistributionNotebook->AddPage("Labelmap S");
    this->IntensityDistributionNotebook->AddPage("Visualization");
    }

  vtkKWFrame *intensity_page = this->IntensityDistributionNotebook->
    GetFrame("Intensity Distribution");
  vtkKWFrame *manual_sampling_page =
    this->IntensityDistributionNotebook->GetFrame("Manual S");
  vtkKWFrame *labelPage =
    this->IntensityDistributionNotebook->GetFrame("Labelmap S");
  vtkKWFrame *gaussianPage =
    this->IntensityDistributionNotebook->GetFrame("Visualization");

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

  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeIntensityDistributionsCallback();

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
      labelPage);
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
    this->LabelmapButton->SetParent(labelPage);
    this->LabelmapButton->Create();
    this->LabelmapButton->SetText("Compute class distribution");
    }

  this->Script(
      "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
      this->LabelmapButton->GetWidgetName());
      
  // Add observer for callbacks    

  this->AddLabelButtonGUIEventsObservers();

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
      SetBalloonHelpString("Select a target image to adjust intensity distribution");

    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    this->Gaussian2DVolumeXMenuButton->GetWidgetName());
  
  this->PopulateGaussian2DVolumeXSelector();    
  
  
  /*if(mrmlManager->GetTargetNumberOfSelectedVolumes() >0){
  this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(0)));
  }*/
        
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
      SetBalloonHelpString("Select a target image to adjust intensity distribution");

    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 5", 
    this->Gaussian2DVolumeYMenuButton->GetWidgetName());
  
  this->PopulateGaussian2DVolumeYSelector();  
    
  /*if(mrmlManager->GetTargetNumberOfSelectedVolumes()>0)
    {
    this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(0)));
    if(mrmlManager->GetTargetNumberOfSelectedVolumes()>1)
      {
      this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(mrmlManager->GetVolumeName(mrmlManager->GetTargetSelectedVolumeNthID(1)));
      }
    }*/
    
/*
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
    

  if (!this->Gaussian2DButton)
    {
    std::cout << "Gaussian2DButton New" << std::endl;
    this->Gaussian2DButton = vtkKWPushButton::New();
    }

  if (!this->Gaussian2DButton->IsCreated())
    {
    std::cout << "Gaussian2DButton->SetParent" << std::endl;
    this->Gaussian2DButton->SetParent(gaussianPage);
    this->Gaussian2DButton->Create();
    this->Gaussian2DButton->SetText("Compute Gaussians");
    }

  this->Script(
      "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
      this->Gaussian2DButton->GetWidgetName());

  this->AddGaussian2DButtonGUIEvents();

  int windowWidth = 300;
  int windowHeight = 300;

  //double x = windowWidth;
  //double y = windowHeight;

  //double sigma = 0.5 * ( x*x + y*y );
  double meanX, meanY, varianceX, varianceY, covariance;

  double rgb[3];

  // Add a render widget, attach it to the view frame, and pack

  if (!this->Gaussian2DWidget)
    {
    this->Gaussian2DWidget = vtkGaussian2DWidget::New();
  
  this->Gaussian2DWidget->SetParent(gaussianPage);
  this->Gaussian2DWidget->Create();
  this->Gaussian2DWidget->GetRenderer()->GetRenderWindow()->GetInteractor()->
    Disable();
  
  meanX = (72)*300/371;
  meanY = (313.67)*300/956;
  varianceX = (25)*90000/(371*371);
  varianceY = (97.333)*90000/(956*956);
  covariance = -(45)*90000/(371*956);
  rgb[0] = 1.0;
  rgb[1] = 0.0;
  rgb[2] = 0.0;
  //this->Gaussian2DWidget->AddGaussian(meanX,meanY,varianceX,varianceY,covariance,rgb);

 /* meanX = 0.75 * (x-1);
  meanY = 0.25 * (y-1);
  rgb[0] = 0.0;
  rgb[1] = 0.0;
  rgb[2] = 1.0;
  this->Gaussian2DWidget->AddGaussian(meanX,meanY,5,33,10.0,rgb);

  meanX = 0.75 * (x-1);
  meanY = 0.75 * (y-1);
  rgb[0] = 0.0;
  rgb[1] = 1.0;
  rgb[2] = 0.0;
  this->Gaussian2DWidget->AddGaussian(meanX,meanY,10,10,1.0,rgb);
*/
  this->Gaussian2DWidget->SetWidth(windowWidth);
  this->Gaussian2DWidget->SetHeight(windowHeight);

  this->Gaussian2DWidget->CreateGaussian2D();
  
  this->Gaussian2DWidget->Render();
  }
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2 -pady 2",this->Gaussian2DWidget
      ->GetWidgetName());

  //std::cout << "numActors " << this->Gaussian2DWidget->GetRenderer()->
  //  GetNumberOfPropsRendered() << std::endl;

  // Deallocate and exit
  ////renderWidget->Delete();
  // GET NUMBER OF LEAF

  // Create the histogram volume selector

  if (!this->IntensityDistributionHistogramButton)
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
    }

  // Create the Gaussian 2D rendering menu button

  if (!this->Gaussian2DRenderingMenuButton)
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
    vtkKWMenu *menu = this->Gaussian2DRenderingMenuButton->GetWidget()->GetMenu();
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  PopulateGaussian2DVolumeXSelector()
{  
  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  vtkKWMenu* menu = this->Gaussian2DVolumeXMenuButton->
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
        this->Gaussian2DVolumeXMenuButton->GetWidget()->SetValue(name);
        }
      }
    }
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  PopulateGaussian2DVolumeYSelector()
{  
  vtkIdType target_vol_id;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  vtkKWMenu* menu = this->Gaussian2DVolumeYMenuButton->
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
        this->Gaussian2DVolumeYMenuButton->GetWidget()->SetValue(name);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
PopulateIntensityDistributionTargetVolumeSelector()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  vtkIdType target_volId;
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
  Gaussian2DVolumeXSelectionChangedCallback(vtkIdType target_volId)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  EMS_DEBUG_MACRO("ShowUserInterface");
  std::cout << __LINE__ << " Gaussian2DVolumeXSelectionChangedCallback"
    <<std::endl;
  std::cout << __LINE__ << " vol id: "<< target_volId <<std::endl;

  //vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(
  //    target_volId)->GetImageData()->GetPointData()->GetScalars();

  //double* range = array->GetRange();

}

//----------------------------------------------------------------------------
/*void vtkEMSegmentIntensityDistributionsStep::
PopulateGaussian2DVolumeXSelector()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  std::cout << __LINE__ << " PopulateGaussian2DVolumeXSelector" << std::endl;
  vtkIdType target_volId;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  vtkKWMenu* menu = this->Gaussian2DVolumeXMenuButton->GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < nb_of_target_volumes; i++)
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
  EMS_DEBUG_MACRO("HideUserInterface");
  this->Superclass::HideUserInterface();
  this->RemoveManualIntensitySamplingGUIObservers();
  this->RemoveGaussian2DButtonGUIEvents();
  this->RemoveLabelButtonGUIEventsObservers();
  this->RemoveLabelSelectorGUIObservers();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentIntensityDistributionsStep::
DisplaySelectedNodeIntensityDistributionsCallback()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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
  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();
  char buffer[256];

  // Update the distribution specification menu button

  if (this->IntensityDistributionSpecificationMenuButton)
    {
    vtkKWMenu *menu = this->IntensityDistributionSpecificationMenuButton->
      GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (hasValidSelection)
      {
      vtksys_stl::string value;
      this->IntensityDistributionSpecificationMenuButton->SetEnabled(enabled);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationManual);
      menu->AddRadioButton("Manual", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId),vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample);
      menu->AddRadioButton("Manual Sampling", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationLabelSample);
      menu->AddRadioButton("Labelmap Sampling", this, buffer);
      sprintf(
        buffer, "IntensityDistributionSpecificationCallback %d %d",
        static_cast<int>(selVolId), vtkEMSegmentMRMLManager::
        DistributionSpecificationAutoSample);
      menu->AddRadioButton("Auto Sampling", this, buffer);

      // temporarily disable auto sampling because it is not currently
      // implemented
      menu->SetItemStateToDisabled("Auto Sampling");

      switch (mrmlManager->GetTreeNodeDistributionSpecificationMethod(
            selVolId))
        {
        case vtkEMSegmentMRMLManager::DistributionSpecificationManual:
          value = "Manual";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationManuallySample:
          value = "Manual Sampling";
          break;
        case vtkEMSegmentMRMLManager::
        DistributionSpecificationLabelSample:
          value = "Label Sampling";
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
    if (hasValidSelection)
      {
      this->IntensityDistributionMeanMatrix->SetEnabled(
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(1);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer, "IntensityDistributionMeanChangedCallback %d", 
        static_cast<int>(selVolId));
      matrix->SetElementChangedCommand(this, buffer);

      for(col = 0; col < nb_of_target_volumes; col++)
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
        nb_of_target_volumes ? enabled : 0);
      matrix->SetNumberOfColumns(nb_of_target_volumes);
      matrix->SetNumberOfRows(nb_of_target_volumes);
      matrix->SetReadOnly(
        mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) !=
        vtkEMSegmentMRMLManager::DistributionSpecificationManual);
      sprintf(
        buffer,"IntensityDistributionCovarianceChangedCallback %d",
        static_cast<int>(selVolId));
      matrix->SetElementChangedCommand(this, buffer);

      for (row = 0; row < nb_of_target_volumes; row++)
        {
        for (col = 0; col < nb_of_target_volumes; col++)
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
        for (col = 0; col < nb_of_target_volumes; col++)
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

}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionSpecificationCallback(
  vtkIdType selVolId, int type)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::Gaussian2DRenderingCallback(
  vtkIdType selVolId, int type)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  // The rendering type has changed because of user interaction
  std::cout << __LINE__ <<
    " vtkEMSegmentIntensityDistributionsStep::Gaussian2DRenderingCallback"
    << std::endl;
  EMS_DEBUG_MACRO("vtkEMSegmentIntensityDistributionsStep::Gaussian2DRenderingCallback");
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
    vtkIdType selVolId, int row, int col, const char *value)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  // The distribution mean vector has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogMean(selVolId, col, atof(value));
}

//-------------------------------------------------------------------
void
vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionCovarianceChangedCallback(
  vtkIdType selVolId, int row, int col, const char *value)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  // The distribution covariance matrix has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeDistributionLogCovariance(selVolId, row,
      col, atof(value));
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
AddIntensityDistributionSamplePoint( double ras[3])
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
PopupManualIntensitySampleContextMenuCallback(int row, int, int x, int y)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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

  if (row >= 0)
    {
    sprintf( buffer, "DeleteManualIntensitySampleCallback %d %d",
      static_cast<int>(selVolId), row);
    this->ContextMenu->AddCommand("Delete sample", this, buffer);
    }

  if (list->GetNumberOfRows())
    {
    sprintf(buffer, "DeleteAllManualIntensitySampleCallback %d",
            static_cast<int>(selVolId));
    this->ContextMenu->AddCommand("Delete all samples", this, buffer);
    }

  this->ContextMenu->PopUp(x, y);
}

//-------------------------------------------------------------------
void
vtkEMSegmentIntensityDistributionsStep::
DeleteManualIntensitySampleCallback( vtkIdType selVolId,
    int sample_index)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  // A sample has been deleted because of user interaction

  if (sample_index >= 0)
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
}

//-------------------------------------------------------------------
void
vtkEMSegmentIntensityDistributionsStep::
DeleteAllManualIntensitySampleCallback(vtkIdType selVolId)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
AddManualIntensitySamplingGUIObservers()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
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
  EMS_DEBUG_MACRO("ShowUserInterface");
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
  EMS_DEBUG_MACRO("ShowUserInterface");
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
void vtkEMSegmentIntensityDistributionsStep::PrintSelf(ostream& os, vtkIndent
    indent)
{
  EMS_DEBUG_MACRO("");
  this->Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetNumberOfLeaf(
  const char *parent, vtkIdType volId)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int numChildren = mrmlManager->GetTreeNodeNumberOfChildren(volId);

  if(numChildren == 0)
  {
  this->NumberOfLeaves = this->NumberOfLeaves +1;
  this->LeafId[this->NumberOfLeaves-1] = static_cast<int>(volId);
  this->LeafLabel[this->NumberOfLeaves-1] = mrmlManager->GetTreeNodeIntensityLabel(this->LeafId[this->NumberOfLeaves-1]);
  }

  for (int i = 0; i < numChildren; i++)
  {
    this->GetNumberOfLeaf( NULL, mrmlManager->GetTreeNodeChildNodeID(volId,
          i));
  }
}
//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionTargetSelectionChangedCallback(vtkIdType targetVolId)
{
  
  EMS_DEBUG_MACRO("IN VOLUME SELECTOR CALLBACK");
  

  double meanX;
  double meanY;
  
  double nomalizationValueX;
  double nomalizationValueY;
  
  double varianceX ;
  double varianceY;
  double covariance;
  
  double rgb[3];
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  
  this->GetGaussian2DTargetVolumes();
  
  nomalizationValueX = /*log*/(this->TargetVolumeXRange[1]-this->TargetVolumeXRange[0]);
  nomalizationValueY = /*log*/(this->TargetVolumeYRange[1]-this->TargetVolumeYRange[0]);
  
  std::cout<< "Normalization Values: " << nomalizationValueX << "  " << nomalizationValueY << std::endl;
  vtkMRMLScene            *mrmlScene   = mrmlManager->GetMRMLScene();
  vtkMRMLColorNode *colorNode = vtkMRMLColorNode::SafeDownCast(mrmlScene->GetNodeByID( mrmlManager->GetColormap() ));
  double *color;
  
  this->Gaussian2DWidget->RemoveAllGaussians();
  
  for(int i=0;i<this->NumberOfLeaves;i++)
    {
    
    color = colorNode->GetLookupTable()->GetTableValue(mrmlManager->GetTreeNodeIntensityLabel(this->LeafId[i]));
    
    rgb[0] =  color[0];
    rgb[1] =  color[1];
    rgb[2] =  color[2];
    
    meanX = mrmlManager->GetTreeNodeDistributionMean(this->LeafId[i],this->TargetVolumeX);
    meanY = mrmlManager->GetTreeNodeDistributionMean(this->LeafId[i],this->TargetVolumeY);
       
    varianceX = mrmlManager->GetTreeNodeDistributionCovariance(
              this->LeafId[i], this->TargetVolumeX, this->TargetVolumeX);
    varianceY = mrmlManager->GetTreeNodeDistributionCovariance(
              this->LeafId[i], this->TargetVolumeY, this->TargetVolumeY);
    covariance = mrmlManager->GetTreeNodeDistributionCovariance(
              this->LeafId[i], this->TargetVolumeX, this->TargetVolumeY);
              
    this->Gaussian2DWidget->AddGaussian(((meanX)*300)/(nomalizationValueX),(((meanY)*300))/(nomalizationValueY),((varianceX)*90000)/(nomalizationValueX*nomalizationValueX),((varianceY)*90000)/(nomalizationValueY*nomalizationValueY),((covariance)*90000)/(nomalizationValueX*nomalizationValueY),rgb); 
    
    std::cout << "meanX: " << meanX << std::endl;
    std::cout << "meanY: " << meanY << std::endl; 
    std::cout << "varianceX: " << varianceX << std::endl; 
    std::cout << "varianceY: " << varianceY << std::endl;   
    std::cout << "covariance: " << covariance << std::endl; 
    std::cout << "RGB: " << rgb[0] << " " << rgb[1] << " " << rgb[2] << std::endl;  
              
   }
   
   this->Gaussian2DWidget->CreateGaussian2D();
   this->Gaussian2DWidget->Render();
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessGaussian2DButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{

  if(event == vtkKWPushButton::InvokedEvent && caller == this->Gaussian2DButton)
  {
    EMS_DEBUG_MACRO("IN PROCESS GAUSSIAN GUI EVENT");

  }
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddGaussian2DButtonGUIEvents()
{
  EMS_DEBUG_MACRO("");
  std::cout << __LINE__ << " AddGaussian2DButtonGUIEvents" << std::endl;

  this->Gaussian2DButton->AddObserver(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveGaussian2DButtonGUIEvents()
{
  EMS_DEBUG_MACRO("");
  std::cout << __LINE__ << " RemoveGaussian2DButtonGUIEvents" << std::endl;
  this->Gaussian2DButton->RemoveObservers(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetGaussian2DTargetVolumes()
{
  vtkIdType target_volId;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  int nb_of_target_volumes = mrmlManager->GetTargetNumberOfSelectedVolumes();

  for(int i = 0; i < nb_of_target_volumes; i++)
    {
    target_volId = mrmlManager->GetTargetSelectedVolumeNthID(i);
    const char *name = mrmlManager->GetVolumeName(target_volId);

    //std::cout<<"NAME: "<<name<<std::endl;
    //std::cout<<"X: "<<this->Gaussian2DVolumeXMenuButton->GetWidget()->GetValue()<<std::endl;
    //std::cout<<"Y: "<<this->Gaussian2DVolumeYMenuButton->GetWidget()->GetValue()<<std::endl;
    
    vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(
      target_volId)->GetImageData()->GetPointData()->GetScalars();

    double* range = array->GetRange();
    //std::cout<<"range: "<<range<<std::endl;
    
    if(strcmp(this->Gaussian2DVolumeXMenuButton->GetWidget()->GetValue(),name) == 0)
      {
      //std::cout<<"COMP SUCCEED IN X"<<std::endl;
      this->TargetVolumeX=i;
      this->TargetVolumeXRange=range;
      }
    if(strcmp(this->Gaussian2DVolumeYMenuButton->GetWidget()->GetValue(),name) == 0)
      {
      //std::cout<<"COMP SUCCEED IN Y"<<std::endl;
      this->TargetVolumeY=i;
      this->TargetVolumeYRange=range;
      }
    }
}
//-------------------------------------------------------------------

void vtkEMSegmentIntensityDistributionsStep::AddLabelButtonGUIEventsObservers()
{

  this->LabelmapButton->AddObserver(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveLabelButtonGUIEventsObservers()
{

  this->LabelmapButton->RemoveObservers(vtkKWPushButton::InvokedEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessLabelButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{

  if(event == vtkKWPushButton::InvokedEvent && caller == this->LabelmapButton)
  {
    EMS_DEBUG_MACRO("IN PROCESS LABEL GUI EVENT");
    
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    
    vtkMRMLScalarVolumeNode *labelVolume =  mrmlManager->GetOutputVolumeNode();
    
    if (labelVolume == NULL)
      {
      vtkErrorMacro("No label volume found");
      return;
      }
      
    // Estimate mean and covariance
    
  // Get the target volumes
  
  int numTargetImages = mrmlManager->GetTargetNumberOfSelectedVolumes();
  
  int labelExtent[6];
  labelVolume->GetImageData()->GetWholeExtent(labelExtent);
  
  std::cout<< "Extent: " << labelExtent[0] << "::" << labelExtent[1] << std::endl;
  std::cout<< "Extent: " << labelExtent[2] << "::" << labelExtent[3] << std::endl;
  std::cout<< "Extent: " << labelExtent[4] << "::" << labelExtent[5] << std::endl;
  
  int sizeI = labelExtent[1] - labelExtent[0] + 1;
  int sizeJ = labelExtent[3] - labelExtent[2] + 1;
  int sizeK = labelExtent[5] - labelExtent[4] + 1;
  
  std::cout<< "sizeI: " << sizeI << std::endl;
  std::cout<< "sizeJ: " << sizeJ << std::endl;
  std::cout<< "sizeK: " << sizeK << std::endl;
  
  vtkIdType volumeID;
  int extent[6];
  
  for (int m=0;m<numTargetImages;m++)
  {
     volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);
     vtkMRMLVolumeNode* targetVolume = mrmlManager->GetVolumeNode(volumeID);
     targetVolume->GetImageData()->GetWholeExtent(extent);

     if (extent[0] != labelExtent[0] || extent[1] != labelExtent[1] ||
         extent[2] != labelExtent[2] || extent[3] != labelExtent[3] ||
         extent[4] != labelExtent[4] || extent[5] != labelExtent[5])
     {
       vtkErrorMacro("Label map has whole extent " << labelExtent[0] << " " <<
       labelExtent[1] << " " << labelExtent[2] << " " << labelExtent[3] << " "
       << labelExtent[4] << " " << labelExtent[5] << " while volume " << volumeID
       << " has whole extent " << extent[0] << " " << extent[1] << " " <<
       extent[2] << " " << extent[3] << " " << extent[4] << " " << extent[5]);
       return;
     }
  }
  
  int component = 0;
  
  int label;
  
  
  //check sizes
  
  // structure of the array : [nbOfTargetImages][totalIntensities][nbOfPoints]
  double meanIntensity[numTargetImages][this->NumberOfLeaves][2];
  
  //initialization of the array
  
  for (int l=0;l<this->NumberOfLeaves;l++)
    {
      for (int m=0;m<numTargetImages;m++)
        {
         meanIntensity[m][l][0] = 0;
         meanIntensity[m][l][1] = 0;
         }
     }

  if ( labelVolume->GetImageData()->GetScalarType() != VTK_SHORT )
  {
    vtkErrorMacro("Label map must have type short. Label map has type "
    << labelVolume->GetImageData()->GetScalarTypeAsString());
    return;
  }

  if ( labelVolume->GetImageData()->GetNumberOfScalarComponents() != 1 )
  {
    vtkErrorMacro("Label map must have voxels with 1 component. The voxels in the label map have "
    << labelVolume->GetImageData()->GetNumberOfScalarComponents() << " components.");
    return;
  }

  short *ptr = (short *)labelVolume->GetImageData()->GetScalarPointer();

  for (int k=0, voxelId=0; k<sizeK; k++)
    {
    for (int j=0; j<sizeJ; j++)
      {
      for (int i=0; i<sizeI; i++, voxelId++)
      {
        label = ptr[voxelId];
        
        if (label != 0)  
          {
          for (int l=0;l<this->NumberOfLeaves;l++)
            {
            
            if (label == this->LeafLabel[l])
              {
              for (int m=0;m<numTargetImages;m++)
                {
                volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);
                //const char* mrmlID = this->MapVTKNodeIDToMRMLNodeID(volumeID);
                vtkMRMLVolumeNode* targetVolume = mrmlManager->GetVolumeNode(volumeID);
                meanIntensity[m][l][0] += targetVolume->GetImageData()->GetScalarComponentAsDouble(i,j,k,component);
                std::cout << __LINE__ << " volume " << m << " labelId " << l << " intensity(" << i << " " << j << " " << k << ") "
                << targetVolume->GetImageData()->GetScalarComponentAsDouble(i,j,k,component) << std::endl;
                meanIntensity[m][l][1] += 1.0;
                }
              }
              
            }
          //std::cout << "Label: " << label << std::endl;
          } 
          
        //std::cout << "Label: " << label << std::endl; 
        label = 0; 
        
        }
      }
    }
    
  for (int i=0;i<numTargetImages;i++)
    {
    for (int j=0;j<this->NumberOfLeaves;j++)
      {
      meanIntensity[i][j][0] = meanIntensity[i][j][0]/meanIntensity[i][j][1] ;
      std::cout << "volume: " << i+1 << std::endl;
      std::cout << "class: " << j+1 << std::endl;
      std::cout << "mean values: " << meanIntensity[i][j][0] << std::endl;
      std::cout << "nb of points: " << meanIntensity[i][j][1] << std::endl;
      }
    }
  
  int maxNumberOfPoints = 0;  
  
  for (int j=0; j<this->NumberOfLeaves; j++)
      {
      if (meanIntensity[0][j][1] > maxNumberOfPoints)
        {
        maxNumberOfPoints = (int)(meanIntensity[0][j][1] + 0.5);
        }
      }
  
  // structure of the array : [nbOfTargetImages][totalIntensities][nbOfPoints]
  double sampleIntensity[numTargetImages][this->NumberOfLeaves][maxNumberOfPoints];
  
  // initialisation of the array
    for (int l=0;l<this->NumberOfLeaves;l++)
    {
      for (int m=0;m<numTargetImages;m++)
        {
        for (int n=0; n<maxNumberOfPoints; n++)
          {
          sampleIntensity[m][l][n] = 0.0;
          }
        }
      }
  
  // get intensity of each point for each class and each image
  for (int k=0, voxelId=0; k<sizeK; k++)
    {
    for (int j=0; j<sizeJ; j++)
      {
      for (int i=0; i<sizeI; i++, voxelId++)
      {
      
        label = ptr[voxelId]; 
        if (label != 0)  
          {
          for (int l=0;l<this->NumberOfLeaves;l++)
            {
            
            if (label == this->LeafLabel[l])
              {
              for (int m=0;m<numTargetImages;m++)
                {
                int n = 0;
                while ( (sampleIntensity[m][l][n]) != (0.0))
                  {
                  //std::cout <<
                   n++;
                  }
                volumeID = mrmlManager->GetTargetSelectedVolumeNthID(m);
                vtkMRMLVolumeNode* targetVolume = mrmlManager->GetVolumeNode(volumeID);
                sampleIntensity[m][l][n] = targetVolume->GetImageData()->GetScalarComponentAsDouble(i,j,k,component) + 1;
                std::cout << __LINE__ << " volume " << m  << "class " << l << " point " << n << " value(" << i << " " << j << " " << k << ") "
                << sampleIntensity[m][l][n] << std::endl;
                n = 0;
                }
              }
              
            }
          } 
          
        label = 0; 
        
        }
      }
    } 
  
  // create covariance matrices
  double cov[numTargetImages][numTargetImages][this->NumberOfLeaves];
  
  //initiallization of the array
  
  for (int l=0;l<this->NumberOfLeaves;l++)
  {
    for (int m=0;m<numTargetImages;m++)
      {
      for (int n=0; n<numTargetImages; n++)
        {
        cov[m][n][l] = 0.0;
        }
      }
    }
  
  // Get covariance
  for (int k=0; k<this->NumberOfLeaves; k++)
    {
    int numberOfSamples = (int)(meanIntensity[0][k][1] + 0.5);
    std::cout << "number of samples " << numberOfSamples << std::endl;
    for (int i=0;i<numTargetImages;i++)
      {
      for (int j=0;j<numTargetImages;j++)
        {
        for (int l=0; l<numberOfSamples; l++)
            {
            cov[i][j][k] += 
              (sampleIntensity[i][k][l] - meanIntensity[i][k][0]) * 
              (sampleIntensity[j][k][l] - meanIntensity[j][k][0]);
              //if (i != j ){ std::cout << "cov: " << cov[i][j][k] << " SI " << sampleIntensity[i][k][l] << " MI " << meanIntensity[i][k][0] << std::endl; }
            }
         }  
      }
      
      for (int i=0;i<numTargetImages;i++)
        {
        for (int j=0;j<numTargetImages;j++)
          {        
          cov[i][j][k] /= numberOfSamples - 1 ;
          std::cout << "covariance " << cov[i][j][k] << std::endl;
          }
        }
        
        numberOfSamples = 0;
    }
  
  
  // fill the array with the new values
  
  for (int i=0; i<this->NumberOfLeaves; i++)
    {
    for (int j=0; j<numTargetImages; j++)
      {
      mrmlManager->SetTreeNodeDistributionLogMean(this->LeafId[i], j, meanIntensity[j][i][0]);
      mrmlManager->SetTreeNodeDistributionMean(this->LeafId[i], j, meanIntensity[j][i][0]);
      for (int k=0; k<numTargetImages; k++)
        {
        mrmlManager->SetTreeNodeDistributionLogCovariance(this->LeafId[i], j,
      k, cov[j][k][i]);
        mrmlManager->SetTreeNodeDistributionCovariance(this->LeafId[i], j,
      k, cov[j][k][i]);
        }
      }
    }

  }
  
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::AddLabelSelectorGUIObservers() 
{
  this->LabelSelector->AddObserver(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::RemoveLabelSelectorGUIObservers()
{
  this->LabelSelector->RemoveObservers(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessLabelSelectorGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
  if (caller == this->LabelSelector && 
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
      this->LabelSelector->GetSelected() != NULL) 
    { 
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    if (mrmlManager)
      {
      mrmlManager->SetOutputVolumeMRMLID(
        this->LabelSelector->GetSelected()->GetID());
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
