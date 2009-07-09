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
  EMS_DEBUG_MACRO("constructor start");
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
  this->IntensityDistributionHistogramHistogram              = NULL;

  this->Gaussian2DWidget                                     = NULL;
  this->NumClassesEntryLabel                                 = NULL;
  this->ClassAndNodeList                                     = NULL;
  this->Gaussian2DButton                                     = NULL;
  this->Gaussian2DRenderingMenuButton                        = NULL;

  this->IntensityDistributionHistogramHistogramVisualization = NULL;
  EMS_DEBUG_MACRO("constructor end");
}

//-------------------------------------------------------------------
vtkEMSegmentIntensityDistributionsStep::
~vtkEMSegmentIntensityDistributionsStep()
{
  EMS_DEBUG_MACRO("destructor start");
  if (this->Gaussian2DButton)
    {
    this->Gaussian2DButton->Delete();
    this->Gaussian2DButton = NULL;
    }

  if (this->ClassAndNodeList)
    {
    this->ClassAndNodeList->Delete();
    this->ClassAndNodeList = NULL;
    }

  if (this->NumClassesEntryLabel)
    {
    this->NumClassesEntryLabel->Delete();
    this->NumClassesEntryLabel = NULL;
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
    this->IntensityDistributionNotebook->AddPage("Manual Sampling");
    this->IntensityDistributionNotebook->AddPage("Gaussian 2D");
    }

  vtkKWFrame *intensity_page = this->IntensityDistributionNotebook->
    GetFrame("Intensity Distribution");
  vtkKWFrame *manual_sampling_page =
    this->IntensityDistributionNotebook->GetFrame("Manual Sampling");
  vtkKWFrame *gaussianPage =
    this->IntensityDistributionNotebook->GetFrame("Gaussian 2D");

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

  // Create the Gaussian 2D page

  std::string msg = "Select an image to define the range of the X axis in ";
  msg += "the 2D gaussian distribution.";

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

  //this->AddGaussian2DButtonGUIEvents();

  int windowWidth = 400;
  int windowHeight = 200;

  double x = windowWidth;
  double y = windowHeight;

  double sigma = 0.5 * ( x*x + y*y );
  double meanX, meanY;

  double rgb[3];

  // Add a render widget, attach it to the view frame, and pack

  if (!this->Gaussian2DWidget)
  {
   this->Gaussian2DWidget = vtkGaussian2DWidget::New();
  }
  this->Gaussian2DWidget->SetParent(gaussianPage);
  this->Gaussian2DWidget->Create();
  this->Gaussian2DWidget->SetWidth(windowWidth);
  this->Gaussian2DWidget->SetHeight(windowHeight);
  this->Gaussian2DWidget->GetRenderer()->GetRenderWindow()->GetInteractor()->
    Disable();
  this->Script("pack %s -expand n -fill none -anchor c",this->Gaussian2DWidget
      ->GetWidgetName());

  meanX = 0.25 * (x-1);
  meanY = 0.75 * (y-1);
  rgb[0] = 1.0;
  rgb[1] = 0.0;
  rgb[2] = 0.0;
  this->Gaussian2DWidget->AddGaussian(meanX,meanY,sigma,rgb);

  meanX = 0.75 * (x-1);
  meanY = 0.25 * (y-1);
  rgb[0] = 0.0;
  rgb[1] = 0.0;
  rgb[2] = 1.0;
  this->Gaussian2DWidget->AddGaussian(meanX,meanY,sigma,rgb);

  meanX = 0.75 * (x-1);
  meanY = 0.75 * (y-1);
  rgb[0] = 0.0;
  rgb[1] = 1.0;
  rgb[2] = 0.0;
  this->Gaussian2DWidget->AddGaussian(meanX,meanY,sigma,rgb);

  this->Gaussian2DWidget->CreateGaussian2D();
  this->Gaussian2DWidget->Render();

  std::cout << "numActors " << this->Gaussian2DWidget->GetRenderer()->
    GetNumberOfPropsRendered() << std::endl;

  // Deallocate and exit
  ////renderWidget->Delete();
  // GET NUMBER OF LEAF

  this->NumberOfLeaves = 0;
  if (mrmlManager)
    {
    vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
    if (root_id)
      {
      this->GetNumberOfLeaf(NULL, root_id);
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
/*
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
        gaussianPage);
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
      PointGuidelineVisibilityOn();
    this->IntensityDistributionHistogramHistogramVisualization->
      SetLabelPositionToTop();
    }

    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
    this->IntensityDistributionHistogramHistogramVisualization->
    GetWidgetName());
*/
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

  //this->AddPointAddGUIEvents();

   // ADD VOLUME SELECTORS FOR CLASSES

  if (!this->ClassAndNodeList)
    {
    this->ClassAndNodeList = vtkKWMultiColumnList::New();
    }

  if (!this->ClassAndNodeList->IsCreated())
    {
    this->ClassAndNodeList->SetParent(gaussianPage);
    this->ClassAndNodeList->Create();
    this->ClassAndNodeList->MovableColumnsOff();

    int col_index;

    col_index = this->ClassAndNodeList->AddColumn("Class");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);
    col_index = this->ClassAndNodeList->AddColumn("Point");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    col_index = this->ClassAndNodeList->AddColumn("Color");
    this->ClassAndNodeList->ColumnEditableOn(col_index);
    this->ClassAndNodeList->SetColumnFormatCommandToEmptyOutput(col_index);
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
void vtkEMSegmentIntensityDistributionsStep::PopulateClassAndNodeList()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  // CLASS COLUMN CREATION
  for(int i=0; i<this->NumberOfLeaves; i++)
  {
    this->ClassAndNodeList->InsertCellTextAsInt(i,1,i+1);
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

//this->IntensityDistributionHistogramHistogramVisualization->ExpandCanvasWidthOn();
//this->IntensityDistributionHistogramHistogramVisualization->SetCanvasHeight(180);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
IntensityDistributionTargetSelectionChangedCallback(vtkIdType targetVolId)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  EMS_DEBUG_MACRO("ShowUserInterface");
  std::cout << __LINE__ <<
    " INTENSITY DISTRIBUTION TARGET SELECTION CHANGED CALLBACK"<<std::endl;
  std::cout<<"target vol id: "<< targetVolId <<std::endl;
  EMS_DEBUG_MACRO("target vol id: ");

  vtkDataArray* array = this->GetGUI()->GetMRMLManager()->GetVolumeNode(
      targetVolId)->GetImageData()->GetPointData()->GetScalars();
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
  this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[0],0.667,1.0,1.0);
  this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(range[1],0.000,1.0,1.0);

  double color;

  for(int i=1; i < this->NumberOfLeaves - 1 ; i++)
  {
  color = 0.66*(1-i/(this->NumberOfLeaves -1));

  std::cout<<"color: "<<color << std::endl;

  this->IntensityDistributionHistogramHistogramFunc->AddHSVPoint(
      (range[0]+range[1])*(i)/(this->NumberOfLeaves-1),color,1.0,1.0);
  }

this->IntensityDistributionHistogramHistogramVisualization->SetDisableAddAndRemove(1);

this->IntensityDistributionHistogramHistogramVisualization->SetColorTransferFunction(this->IntensityDistributionHistogramHistogramFunc);
this->IntensityDistributionHistogramHistogramVisualization->SetWholeParameterRangeToFunctionRange();
this->IntensityDistributionHistogramHistogramVisualization->SetVisibleParameterRangeToWholeParameterRange();
this->IntensityDistributionHistogramHistogramVisualization->ParameterRangeVisibilityOn();

this->IntensityDistributionHistogramHistogramVisualization->ExpandCanvasWidthOn();
this->IntensityDistributionHistogramHistogramVisualization->SetCanvasHeight(180);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::HideUserInterface()
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  this->Superclass::HideUserInterface();
  this->RemoveManualIntensitySamplingGUIObservers();
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
  int hasValidSelection = tree->HasSelection();
  if (hasValidSelection)
    {
    sel_node = tree->GetSelection();
    selVolId = tree->GetNodeUserDataAsInt(sel_node.c_str());
    hasValidSelection = mrmlManager->GetTreeNodeIsLeaf(selVolId);
    manually_sample_mode =
      mrmlManager->GetTreeNodeDistributionSpecificationMethod(selVolId) ==
      vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample;
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
      "Manual Sampling", manually_sample_mode);
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

    std::cout<<"test1: "<<ras[0]<<std::endl;
    std::cout<<"test2: "<<ras[1]<<std::endl;
    std::cout<<"test3: "<<ras[2]<<std::endl;

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
  }

  for (int i = 0; i < numChildren; i++)
  {
    this->GetNumberOfLeaf( NULL, mrmlManager->GetTreeNodeChildNodeID(volId,
          i));
  }
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetParentPercent(
  int i, vtkIdType volId)
{
  EMS_DEBUG_MACRO("");
  this->Depth = this->Depth +1 ;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  vtkIdType parent_ID = mrmlManager->GetTreeNodeParentNodeID(volId);

  if(parent_ID != mrmlManager->GetTreeRootNodeID())
  {
  this->ClassPercentOrder[this->Depth][i] = parent_ID;
  this->ClassPercentOrderCP[this->Depth][i] = parent_ID;
  this->GetParentPercent(i, parent_ID);
  }
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::ProcessGaussian2DButtonGUIEvents(
    vtkObject *caller, unsigned long event, void *callData)
{
  EMS_DEBUG_MACRO("");
  if(event == vtkKWPushButton::InvokedEvent)
  {
    std::cout<<"TEST PERCENT"<<std::endl;
    std::cout<<"NumberOfLeaves "<<this->NumberOfLeaves<<std::endl;

    for(int i=0; i < this->NumberOfLeaves; i++)
    {
      std::cout<<"iteration "<<i<<std::endl;
      std::cout<<"leaf ID "<<this->LeafId[i]<<std::endl;

      this->Depth = 0;
      this->ClassPercentOrder[0][i] = this->LeafId[i];
      this->ClassPercentOrderCP[0][i] = this->LeafId[i];

      this->GetParentPercent(i,this->LeafId[i]);
    }

    int control = 1;
    int position = 0;
    int positionF ;

    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

    for(int i=0; i<this->NumberOfLeaves; i++)
    {
      for(int j=0; j<200; j++)
      {
        for(int k=0; k<this->NumberOfLeaves; k++)
        {
          for(int l=0; l<200; l++)
          {
            if(this->ClassPercentOrder[j][i] ==
                this->ClassPercentOrderCP[l][k] &&
                this->ClassPercentOrder[j][i] != 0)
            {
              this->ClassPercentOrderCP[l][k] = 0;
              // FIND IF POINT EXISTS AND WHERE this->ClassPercentOrder[j][i]
              for(int m = 0; m < 200 ;m++)
              {
                if(this->CorrespondenceArray[0][m] ==
                    this->ClassPercentOrder[j][i])
                {
                  positionF = m;
                  control = 0;
                  std::cout<<"POINT FOUND: "<< positionF <<std::endl;
                }
              }

              if(control != 0)
              {
                positionF = position;
                std::cout<<"POINT NOT FOUND: "<< positionF <<std::endl;
                position++;
              }

              this->CorrespondenceArray[0][positionF] =
                this->ClassPercentOrder[j][i];

              std::cout<<"COMPARE WIDGET AND ARRAY TO GET POSITION "
                << std::endl;

              for(int z=0; z< this->NumberOfLeaves; z++)
              {
                std::cout<<"VALUE OF Z: "<< z << std::endl;
                if(strcmp(this->ClassAndNodeList->GetCellText(z,0),
                      mrmlManager->GetTreeNodeName(this->
                        ClassPercentOrder[0][k])) == 0)
                {
                  this->ClassWeight[positionF] =
                    this->ClassWeight[positionF] + this->GetWeight(z);
                  //remplace 1 by NB_PIX_[Z]
                  std::cout<<"CLASS WEIGHT : "<< this->ClassWeight[positionF]
                    <<std::endl;
                }
              }
              std::cout<<"END LOOP: "<< std::endl;
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
}

//-------------------------------------------------------------------
void vtkEMSegmentIntensityDistributionsStep::GetPercent( int j,
    vtkIdType volId)
{
  EMS_DEBUG_MACRO("");
  this->Depth++;

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  int numChildren = mrmlManager->GetTreeNodeNumberOfChildren(volId);
  double weight = 0.0;

  if(numChildren > 0)
  {
    for (int i=0; i < numChildren; i++)
    {
      for(int m=0; m < 200 ;m++)
      {
        if(this->CorrespondenceArray[0][m] == mrmlManager->
            GetTreeNodeChildNodeID(volId, i))
        {
          weight = weight + this->ClassWeight[m];
        }
      }
    }

    for (int i = 0; i < numChildren; i++)
    {
      for(int m = 0; m < 200 ;m++)
      {
        if(this->CorrespondenceArray[0][m] == mrmlManager->
            GetTreeNodeChildNodeID(volId, i))
        {
          this->ClassWeight[m] = (this->ClassWeight[m])/weight;
        }
      }
    }

    for (int i=0; i < numChildren; i++)
    {
    this->GetPercent(3,mrmlManager->GetTreeNodeChildNodeID(volId, i));
    }

    for (int i=0; i < 200; i++)
    {
      if(this->CorrespondenceArray[0][i] != 0)
      {
        mrmlManager->SetTreeNodeClassProbability(this->
            CorrespondenceArray[0][i], this->ClassWeight[i]);
      }
    }
  }
}

//-------------------------------------------------------------------
double vtkEMSegmentIntensityDistributionsStep::GetWeight(int z)
{
  EMS_DEBUG_MACRO("ShowUserInterface");
  double sum = 0.0;

  int start = (int)(this->ClassSize[z*2  ] + 0.5) + 1;
  int stop  = (int)(this->ClassSize[z*2+1] + 0.5);

  for(int l = start; l < stop; l++)
  {
    sum += this->IntensityDistributionHistogramHistogram->
      GetOccurenceAtValue(l);
  }

  return sum;
}

