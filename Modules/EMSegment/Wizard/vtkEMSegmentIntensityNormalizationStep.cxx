/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentIntensityNormalizationStep.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkEMSegmentIntensityNormalizationStep.h"
#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkAppendPolyData.h"
#include "vtkBoxWidget.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
#include "vtkGlyph3D.h"
#include "vtkInteractorEventRecorder.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"

#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWHistogram.h"
#include "vtkKWLabel.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

// Callback for the interaction
class vtkBWCallback : public vtkCommand
{
public:
  static vtkBWCallback *New()
    {
      return new vtkBWCallback;
    }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      vtkBoxWidget *boxWidget = reinterpret_cast<vtkBoxWidget*>(caller);
      boxWidget->GetTransform(this->Transform);
      this->Actor->SetUserTransform(this->Transform);
    }

  vtkBWCallback():Transform(0),Actor(0) {}
  vtkTransform *Transform;
  vtkActor     *Actor;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentIntensityNormalizationStep);
vtkCxxRevisionMacro(vtkEMSegmentIntensityNormalizationStep,
    "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentIntensityNormalizationStep::
vtkEMSegmentIntensityNormalizationStep()
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

  this->MaskRenderWidget                     = NULL;

  this->NormalizationHistogramFrame          = NULL;
  this->NormalizationHistogram               = NULL;
  this->NormalizationHistogramMenuButton     = NULL;
  this->NormalizationPiecewiseFunctionEditor = NULL;

  this->NormalizationValueRecommendedEntry   = NULL;
  this->RecommendationFrame                  = NULL;

  this->Actor  = vtkActor::New();
  this->Mapper = vtkPolyDataMapper::New();
}

//----------------------------------------------------------------------------
vtkEMSegmentIntensityNormalizationStep::
~vtkEMSegmentIntensityNormalizationStep()
{
  if (this->NormalizationTargetVolumeMenuButton)
  {
    this->NormalizationTargetVolumeMenuButton->Delete();
    this->NormalizationTargetVolumeMenuButton = NULL;
  }

  if (this->NormalizationEnableCheckButton)
  {
    this->NormalizationEnableCheckButton->Delete();
    this->NormalizationEnableCheckButton = NULL;
  }

  if (this->NormalizationDefaultsMenuButton)
  {
    this->NormalizationDefaultsMenuButton->Delete();
    this->NormalizationDefaultsMenuButton = NULL;
  }

  if (this->NormalizationPrintCheckButton)
  {
    this->NormalizationPrintCheckButton->Delete();
    this->NormalizationPrintCheckButton = NULL;
  }

  if (this->NormalizationNormValueEntry)
  {
    this->NormalizationNormValueEntry->Delete();
    this->NormalizationNormValueEntry = NULL;
  }

  if (this->NormalizationSmoothingWidthEntry)
  {
    this->NormalizationSmoothingWidthEntry->Delete();
    this->NormalizationSmoothingWidthEntry = NULL;
  }

  if (this->NormalizationMaxSmoothingWidthEntry)
  {
    this->NormalizationMaxSmoothingWidthEntry->Delete();
    this->NormalizationMaxSmoothingWidthEntry = NULL;
  }

  if (this->NormalizationRelativeMaxVoxelScale)
  {
    this->NormalizationRelativeMaxVoxelScale->Delete();
    this->NormalizationRelativeMaxVoxelScale = NULL;
  }

  if (this->NormalizationParametersFrame)
  {
    this->NormalizationParametersFrame->Delete();
    this->NormalizationParametersFrame = NULL;
  }

  if (this->MaskRenderWidget)
  {
    this->MaskRenderWidget->Delete();
    this->MaskRenderWidget = NULL;
  }

  if (this->NormalizationHistogram)
  {
    this->NormalizationHistogram->Delete();
    this->NormalizationHistogram = NULL;
  }

  if (this->NormalizationHistogramFrame)
  {
    this->NormalizationHistogramFrame->Delete();
    this->NormalizationHistogramFrame = NULL;
  }

  if (this->NormalizationHistogramMenuButton)
  {
    this->NormalizationHistogramMenuButton->Delete();
    this->NormalizationHistogramMenuButton = NULL;
  }

  if (this->NormalizationPiecewiseFunctionEditor)
  {
    this->NormalizationPiecewiseFunctionEditor->Delete();
    this->NormalizationPiecewiseFunctionEditor = NULL;
  }

  if (this->NormalizationValueRecommendedEntry)
  {
    this->NormalizationValueRecommendedEntry->Delete();
    this->NormalizationValueRecommendedEntry = NULL;
  }

  if (this->RecommendationFrame)
  {
    this->RecommendationFrame->Delete();
    this->RecommendationFrame = NULL;
  }

  if (this->Actor)
  {
    this->Actor->Delete();
    this->Actor = NULL;
  }

  if (this->Mapper)
  {
    this->Mapper->Delete();
    this->Mapper = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget =
    this->GetGUI()->GetWizardWidget();

  wizardWidget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizardWidget->GetClientArea();

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
    this->NormalizationTargetVolumeMenuButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH+10);
    this->NormalizationTargetVolumeMenuButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->NormalizationTargetVolumeMenuButton->SetLabelText("Target Image:");
    this->NormalizationTargetVolumeMenuButton->SetBalloonHelpString(
        "Select a target image to set intensity normalization parameters.");
  }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 5",
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

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
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

  vtkKWMenu *menu =
    this->NormalizationTargetVolumeMenuButton->GetWidget()->GetMenu();

  // Create the normalization actions menu button

  if (!this->NormalizationDefaultsMenuButton)
  {
    this->NormalizationDefaultsMenuButton = vtkKWMenuButton::New();
  }

  if (!this->NormalizationDefaultsMenuButton->IsCreated())
  {
    this->NormalizationDefaultsMenuButton->SetParent(parentFrame);
    this->NormalizationDefaultsMenuButton->Create();
    this->NormalizationDefaultsMenuButton->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH-3);
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
    this->NormalizationPrintCheckButton->SetLabelText("Print Info:");
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
    this->NormalizationNormValueEntry->SetLabelText("Normalization Value:");
    this->NormalizationNormValueEntry->SetLabelWidth(
        EMSEG_WIDGETS_LABEL_WIDTH-4);
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
    this->NormalizationSmoothingWidthEntry->SetLabelText(
        "Histogram Smoothing Width:");
    this->NormalizationSmoothingWidthEntry->SetLabelWidth(
        EMSEG_WIDGETS_LABEL_WIDTH -4);

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

    vtkKWEntry *entry = this->NormalizationMaxSmoothingWidthEntry->
      GetWidget();
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
    this->NormalizationRelativeMaxVoxelScale->SetLabelText(
        "Relative Max Voxel Number:");
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
      mrmlManager0->GetTargetNumberOfSelectedVolumes() ?
      parent->GetEnabled() : 0);

  if (this->NormalizationTargetVolumeMenuButton->GetEnabled())
  {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

    if (!mrmlManager->GetTargetNumberOfSelectedVolumes() ||
        menu->GetNumberOfItems()<=0)
    {
      this->NormalizationTargetVolumeMenuButton->GetWidget()->SetValue("");
    }
    else
    {
      menu->SelectItem(0);
      this->NormalizationTargetSelectionChangedCallback(mrmlManager->
          GetTargetSelectedVolumeNthID(0));
    }
  }

  //--------------------------------------------------------------------------
  // Create the mask render widget

  if (!this->MaskRenderWidget)
  {
    this->MaskRenderWidget = vtkKWRenderWidget::New();
  }

  if (!this->MaskRenderWidget->IsCreated())
  {
    this->MaskRenderWidget->SetParent(parent);
    this->MaskRenderWidget->Create();
  }

  //this->RunRenderWidget->SetEnabled(
    //mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
    this->MaskRenderWidget->GetWidgetName());

  //--------------------------------------------------------------------------
  // Create the histogram frame

  if (!this->NormalizationHistogramFrame)
  {
    this->NormalizationHistogramFrame = vtkKWFrameWithLabel::New();
  }

  if (!this->NormalizationHistogramFrame->IsCreated())
  {
    this->NormalizationHistogramFrame->SetParent(parent);
    this->NormalizationHistogramFrame->Create();
    this->NormalizationHistogramFrame->SetLabelText("Histogram");
  }

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
    this->NormalizationHistogramFrame->GetWidgetName());

  vtkKWFrame* userFrame = this->NormalizationHistogramFrame->GetFrame();

  // Create the histogram volume selector

  if (!this->NormalizationHistogramMenuButton)
    {
    this->NormalizationHistogramMenuButton = vtkKWMenuButtonWithLabel::New();
    }

  if (!this->NormalizationHistogramMenuButton->IsCreated())
    {
    this->NormalizationHistogramMenuButton->SetParent(userFrame);//parent
    this->NormalizationHistogramMenuButton->Create();
    this->NormalizationHistogramMenuButton->GetWidget()->SetWidth(
        EMSEG_MENU_BUTTON_WIDTH+10);
    this->NormalizationHistogramMenuButton->GetLabel()->SetWidth(
        EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->NormalizationHistogramMenuButton->SetLabelText("Atlas Image:");
    this->NormalizationHistogramMenuButton->SetBalloonHelpString(
        "Select a target image find intensity normalization parameters.");
    }

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 5",
    this->NormalizationHistogramMenuButton->GetWidgetName());

  this->PopulateNormalizationHistogramSelector();

  /*
  if (mrmlManager0->GetTargetNumberOfSelectedVolumes()>0)
    {
    this->NormalizationHistogramMenuButton->GetWidget()->SetValue(mrmlManager0
    ->GetVolumeName(mrmlManager0->GetVolumeNthID(0)));
    }
  */

  // Create the histogram

  if (!this->NormalizationHistogram)
    {
    this->NormalizationHistogram = vtkKWHistogram::New();
    this->NormalizationPiecewiseFunctionEditor = vtkKWPiecewiseFunctionEditor
      ::New();
    }

  if (!this->NormalizationPiecewiseFunctionEditor->IsCreated())
    {
    this->NormalizationPiecewiseFunctionEditor->SetParent(userFrame);//parent
    this->NormalizationPiecewiseFunctionEditor->Create();
    this->NormalizationPiecewiseFunctionEditor->GetTclName();
    this->NormalizationPiecewiseFunctionEditor->SetBorderWidth(2);
    this->NormalizationPiecewiseFunctionEditor->SetPadX(2);
    this->NormalizationPiecewiseFunctionEditor->SetPadY(2);

    this->NormalizationPiecewiseFunctionEditor->ParameterTicksVisibilityOn();
    this->NormalizationPiecewiseFunctionEditor->ValueTicksVisibilityOn();
    this->NormalizationPiecewiseFunctionEditor->
      ComputeValueTicksFromHistogramOn();
    this->NormalizationPiecewiseFunctionEditor->SetParameterTicksFormat(
        "%-#6.0f");
    this->NormalizationPiecewiseFunctionEditor->SetValueTicksFormat(
    this->NormalizationPiecewiseFunctionEditor->GetParameterTicksFormat());
    this->NormalizationPiecewiseFunctionEditor->SetFrameBackgroundColor(0.92,
        1.0, 0.92);
    this->NormalizationPiecewiseFunctionEditor->SetParameterCursorVisibility(
        50);
    this->NormalizationPiecewiseFunctionEditor->SetParameterCursorPosition(
        50);
    this->NormalizationPiecewiseFunctionEditor->
      SetParameterCursorInteractionStyle(7);
    }

  this->Script(
      "pack %s -side top -anchor nw -fill both -padx 2 -pady 2 -pady 2",
      this->NormalizationPiecewiseFunctionEditor->GetWidgetName());

  this->NormalizationHistogramMenuButton->SetEnabled(mrmlManager0->
      GetVolumeNumberOfChoices() ? parent->GetEnabled() : 0);

  if (this->NormalizationHistogramMenuButton->GetEnabled())
    {
    // Select the target volume, and update everything else accordingly
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    int volId = mrmlManager->GetVolumeNthID(0);
    this->NormalizationHistogramChangedCallback(volId);
    }

  this->AddCursorMovingGUIEvents();

  // Create the recommendation frame

  if (!this->RecommendationFrame)
    {
    this->RecommendationFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->RecommendationFrame->IsCreated())
    {
    this->RecommendationFrame->SetParent(parent);
    this->RecommendationFrame->Create();
    this->RecommendationFrame->SetBackgroundColor(1,0.56,0);
    this->RecommendationFrame->SetLabelText("Recommendations");
    }

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
    this->RecommendationFrame->GetWidgetName());

  vtkKWFrame* recommendationFrame = this->RecommendationFrame->GetFrame();

  // Create the recommended value

  if (!this->NormalizationValueRecommendedEntry)
    {
    this->NormalizationValueRecommendedEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->NormalizationValueRecommendedEntry->IsCreated())
    {
    this->NormalizationValueRecommendedEntry->SetParent(recommendationFrame);
    this->NormalizationValueRecommendedEntry->Create();
    this->NormalizationValueRecommendedEntry->SetLabelText(
        "Recommended Normalization Value:");
    this->NormalizationValueRecommendedEntry->SetLabelWidth(
        EMSEG_WIDGETS_LABEL_WIDTH -8);

    vtkKWEntry *entry = this->NormalizationValueRecommendedEntry->GetWidget();
    entry->SetWidth(6);
    entry->SetRestrictValueToInteger();
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2",
      this->NormalizationValueRecommendedEntry->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  PopulateNormalizationTargetVolumeSelector()
{
  vtkIdType targetVolId;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

  vtkKWMenu* menu = this->NormalizationTargetVolumeMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numTargets; i++)
    {
    targetVolId = mrmlManager->GetTargetSelectedVolumeNthID(i);

    sprintf(buffer, "%s %d", "NormalizationTargetSelectionChangedCallback",
            static_cast<int>(targetVolId));

    const char *name = mrmlManager->GetVolumeName(targetVolId);

    if (name)
      {
      menu->AddRadioButton(name, this, buffer);

      if (i == 0)
        {
        this->NormalizationTargetVolumeMenuButton->GetWidget()->
          SetValue(name);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  PopulateNormalizationHistogramSelector()
{
  vtkIdType volId;
  char buffer[256];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

 if (!mrmlManager)
    {
    return;
    }

 int numVolumes = mrmlManager->GetVolumeNumberOfChoices();

  vtkKWMenu* menu = this->NormalizationHistogramMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  // Update the target volume list in the menu button

  for(int i = 0; i < numVolumes; i++)
    {
    volId = mrmlManager->GetVolumeNthID(i);
    sprintf(buffer,"%s %d","NormalizationHistogramChangedCallback",
        static_cast<int>(volId));
    const char *name = mrmlManager->GetVolumeName(volId);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      if (i == 0)
        {
        this->NormalizationHistogramMenuButton->GetWidget()->SetValue(name);
        }
      }
    }

}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationTargetSelectionChangedCallback(vtkIdType targetVolId)
{
  // The target volumes have changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  char buffer[256];
  int parentEnabled = this->NormalizationParametersFrame->GetEnabled();

  // Update Normalization-Enable check button

  this->NormalizationEnableCheckButton->SetEnabled(parentEnabled);
  vtkKWCheckButton *cbEnable =
    this->NormalizationEnableCheckButton->GetWidget();
  sprintf(buffer, "NormalizationEnableCallback %d",
      static_cast<int>(targetVolId));
  cbEnable->SetCommand(this, buffer);
  cbEnable->SetSelectedState(mrmlManager->
    GetTargetVolumeIntensityNormalizationEnabled(targetVolId));
  this->NormalizationEnableCallback(
    targetVolId, cbEnable->GetSelectedState());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationHistogramChangedCallback(vtkIdType targetVolId)
{
  // The target volumes have changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(targetVolId);

  vtkImageData* inputImage = volumeNode->GetImageData();

  vtkDataArray* array = inputImage->GetPointData()->GetScalars();

  this->NormalizationHistogram->BuildHistogram(array,0);
  this->NormalizationPiecewiseFunctionEditor->SetHistogram(
      NormalizationHistogram);
  this->NormalizationPiecewiseFunctionEditor->DisplayHistogramOnly();

  this->NormalizationPiecewiseFunctionEditor->
    SetWholeParameterRangeToFunctionRange();
  this->NormalizationPiecewiseFunctionEditor->
    SetVisibleParameterRangeToWholeParameterRange();
  this->NormalizationPiecewiseFunctionEditor->ParameterRangeVisibilityOn();
  this->NormalizationPiecewiseFunctionEditor->ExpandCanvasWidthOn();
  this->NormalizationPiecewiseFunctionEditor->SetCanvasHeight(180);

  this->IdEvent = targetVolId;
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationEnableCallback(vtkIdType targetVolId, int checked)
{
  // The target volume enabled checkbutton has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }

  mrmlManager->SetTargetVolumeIntensityNormalizationEnabled(targetVolId,
      checked);

  int isEnabled = this->NormalizationParametersFrame->GetEnabled();
  if(!checked)
    {
    isEnabled = 0;
    }
  this->NormalizationDefaultsMenuButton->SetEnabled(isEnabled);
  this->NormalizationPrintCheckButton->SetEnabled(isEnabled);
  this->NormalizationNormValueEntry->SetEnabled(isEnabled);
  this->NormalizationSmoothingWidthEntry->SetEnabled(isEnabled);
  this->NormalizationMaxSmoothingWidthEntry->SetEnabled(isEnabled);
  this->NormalizationRelativeMaxVoxelScale->SetEnabled(isEnabled);

  if(!isEnabled)
    {
    return;
    }

  // Update Normalization default settings drop down
  char buffer[256];

  this->NormalizationDefaultsMenuButton->GetMenu()->DeleteAllItems();
  sprintf(buffer, "NormalizationNormTypeCallback %d %d",
      static_cast<int>(targetVolId), NormalizationDefaultT1SPGR);
  this->NormalizationDefaultsMenuButton->
    GetMenu()->AddRadioButton("MR T1 SPGR", this, buffer);
  sprintf(buffer, "NormalizationNormTypeCallback %d %d",
      static_cast<int>(targetVolId), NormalizationDefaultT2);
  this->NormalizationDefaultsMenuButton->GetMenu()->AddRadioButton("MR T2",
      this, buffer);

  // Update Print-Info check button

  sprintf(buffer, "NormalizationPrintInfoCallback %d",
      static_cast<int>(targetVolId));
  this->NormalizationPrintCheckButton->GetWidget()->SetCommand(this, buffer);

  // Update Norm-value entry

  vtkKWEntry *entry = this->NormalizationNormValueEntry->GetWidget();
  sprintf(buffer, "NormalizationValueCallback %d",
          static_cast<int>(targetVolId));
  entry->SetCommand(this, buffer);

  // Update Histogram-smoothing-width

  entry = this->NormalizationSmoothingWidthEntry->GetWidget();
  sprintf(buffer, "NormalizationSmoothingWidthCallback %d",
      static_cast<int>(targetVolId));
  entry->SetCommand(this, buffer);

  // Update Max Histogram-smoothing-width

  entry = this->NormalizationMaxSmoothingWidthEntry->GetWidget();
  sprintf(buffer, "NormalizationMaxSmoothingWidthCallback %d",
      static_cast<int>(targetVolId));
  entry->SetCommand(this, buffer);

  // Update the Relative-Max-Voxel Num scale entry.

  sprintf( buffer, "NormalizationRelativeMaxVoxelNumCallback %d",
    static_cast<int>(targetVolId));
  this->NormalizationRelativeMaxVoxelScale->SetEndCommand(this, buffer);
  this->NormalizationRelativeMaxVoxelScale->SetEntryCommand(this, buffer);

  this->ResetDefaultParameters(targetVolId);

  int parentEnabled = this->NormalizationParametersFrame->GetEnabled();
  vtkKWCheckButton *cbEnable = this->NormalizationEnableCheckButton->
    GetWidget();

  this->NormalizationDefaultsMenuButton->SetEnabled(
    cbEnable->GetSelectedState() ? parentEnabled : 0);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationNormTypeCallback(vtkIdType targetVolId,
  int enumDefaultsetting)
{
  // The target volume default settings dropdown has changed because of user
  // interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  if (enumDefaultsetting == NormalizationDefaultT1SPGR)
    {
    mrmlManager->SetTargetVolumeIntensityNormalizationToDefaultT1SPGR(
      targetVolId);
    }
  else if (enumDefaultsetting == NormalizationDefaultT2)
    {
    mrmlManager->SetTargetVolumeIntensityNormalizationToDefaultT2(
      targetVolId);
    }
  this->ResetDefaultParameters(targetVolId);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::NormalizationPrintInfoCallback(
  vtkIdType targetVolId, int checked)
{
  // The print-info checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  mrmlManager->SetTargetVolumeIntensityNormalizationPrintInfo( targetVolId,
      checked);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationValueCallback(vtkIdType targetVolId, double dValue)
{
  // The Norm-value entry has changed because of user interaction
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  mrmlManager->SetTargetVolumeIntensityNormalizationNormValue(targetVolId,
      dValue);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
  NormalizationSmoothingWidthCallback(vtkIdType targetVolId, int iValue)
{
  // The Norm-value entry has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  mrmlManager->
    SetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(
    targetVolId, iValue);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
NormalizationMaxSmoothingWidthCallback(vtkIdType targetVolId, int iValue)
{
  // The Norm-value entry has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  mrmlManager->
    SetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(
    targetVolId, iValue);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::
NormalizationRelativeMaxVoxelNumCallback(vtkIdType targetVolId,double
    dValue)
{
  // The Norm-value entry has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  mrmlManager->SetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(
    targetVolId, dValue);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ResetDefaultParameters(vtkIdType
    targetVolId)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
  {
    return;
  }

  this->NormalizationDefaultsMenuButton->SetConfigurationOption("-text",
      "Reset Defaults");

  this->NormalizationPrintCheckButton->GetWidget()->SetSelectedState(
      mrmlManager->GetTargetVolumeIntensityNormalizationPrintInfo(
        targetVolId));
  this->NormalizationNormValueEntry->GetWidget()->SetValueAsDouble(
      mrmlManager->GetTargetVolumeIntensityNormalizationNormValue(
        targetVolId));
  this->NormalizationSmoothingWidthEntry->GetWidget()->SetValueAsInt(
      mrmlManager->
      GetTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(
        targetVolId));
  this->NormalizationMaxSmoothingWidthEntry->GetWidget()->SetValueAsInt(
      mrmlManager->
      GetTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(
        targetVolId));
  this->NormalizationRelativeMaxVoxelScale->SetValue(mrmlManager->
      GetTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(
        targetVolId));
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::PrintSelf(ostream& os, vtkIndent
    indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::GetHistogramValue()
{
  std::cout<<__LINE__<<" vtkEMSegementIntensityNormalizationStep Position1: "
    <<this->NormalizationPiecewiseFunctionEditor->
    GetParameterCursorPosition()<<std::endl;
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::ProcessCursorMovingGUIEvents(
  vtkObject *vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData))
{
  if (event == vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent)
  {
    double range[2];
    double occurence;
    double counter = 0;
    double mean = 0;

    this->NormalizationHistogram->GetRange(range);
    occurence = this->NormalizationHistogram->GetOccurenceAtValue(90);

    for (double i = this->NormalizationPiecewiseFunctionEditor->
        GetParameterCursorPosition(); i < range[1] ; i++)
    {
      double num = this->NormalizationHistogram->GetOccurenceAtValue(i);
      counter += num;
      mean += i * num;
    }

    if (counter)
    {
      mean /= counter;
    }

    this->NormalizationValueRecommendedEntry->GetWidget()->SetValueAsDouble(
        (int)(mean+0.5));
  }
  else if (this->Mapper->GetInput() == NULL)
  {
    if (this->MaskRenderWidget->GetRenderWindow() == NULL)
    {
      vtkEMSegmentMRMLManager *mrmlManager =
        this->GetGUI()->GetMRMLManager();

      vtkMRMLScalarVolumeNode *labelVolume =
        mrmlManager->GetOutputVolumeNode();

      if (labelVolume == NULL)
      {
        vtkErrorMacro("No label volume found");
        return;
      }

      if (labelVolume->GetImageData() == NULL)
      {
        vtkErrorMacro("Label Map has no data");
        return;
      }

      vtkRenderer *renderer = this->MaskRenderWidget->GetRenderer();

      vtkRenderWindow *renWin = this->MaskRenderWidget->GetRenderWindow();

      vtkRenderWindowInteractor *iren = renWin->GetInteractor();

      vtkBoxWidget *boxWidget = vtkBoxWidget::New();
      boxWidget->SetInteractor( iren );
      boxWidget->SetPlaceFactor( 1.25 );

      vtkConeSource *cone = vtkConeSource::New();
      cone->SetResolution(6);

      vtkSphereSource *sphere = vtkSphereSource::New();
      sphere->SetThetaResolution(8); sphere->SetPhiResolution(8);

      vtkGlyph3D *glyph = vtkGlyph3D::New();
      glyph->SetInputConnection(sphere->GetOutputPort());
      glyph->SetSource(cone->GetOutput());
      glyph->SetVectorModeToUseNormal();
      glyph->SetScaleModeToScaleByVector();
      glyph->SetScaleFactor(0.25);

      vtkAppendPolyData *append = vtkAppendPolyData::New();
      append->AddInput(glyph->GetOutput());
      append->AddInput(sphere->GetOutput());

      this->Mapper->SetInput(append->GetOutput());

      this->Actor->SetMapper(this->Mapper);

      this->MaskRenderWidget->RemoveAllViewProps();
      this->MaskRenderWidget->AddViewProp(this->Actor);

      renderer->SetBackground(0,0,0);
      renWin->SetSize(400,400);

      // Configure the box widget including callbacks
      vtkTransform *transform = vtkTransform::New();
      boxWidget->SetProp3D(this->Actor);
      boxWidget->PlaceWidget();

      vtkBWCallback *myCallback = vtkBWCallback::New();
      myCallback->Transform = transform;

      myCallback->Actor = this->Actor;
      boxWidget->AddObserver(vtkCommand::InteractionEvent,myCallback);

      // record events
      //vtkInteractorEventRecorder *recorder =
      //vtkInteractorEventRecorder::New();
      //recorder->SetInteractor(iren);
      //recorder->SetFileName("c:/record.log");
      //recorder->Record();
      //recorder->ReadFromInputStringOn();
      //recorder->SetInputString(BoxWidgetEventLog);

      this->MaskRenderWidget->Render();
      //recorder->Play();

      // Remove the observers so we can go interactive. Without this the "-I"
      // testing option fails.
      //recorder->Off();

      //int retVal = vtkRegressionTestImage( renWin );
      //if ( retVal == vtkRegressionTester::DO_INTERACTOR)
      //{
      //iren->Start();
      //}
/*
      t->Delete();
      myCallback->Delete();
      boxWidget->Delete();
      sphere->Delete();
      cone->Delete();
      glyph->Delete();
      append->Delete();
*/
    }
  }
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::AddCursorMovingGUIEvents()
{
  this->NormalizationPiecewiseFunctionEditor->AddObserver(
      vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::RemoveCursorMovingGUIEvents()
{
  this->NormalizationPiecewiseFunctionEditor->RemoveObservers(
      vtkKWPiecewiseFunctionEditor::ParameterCursorMovingEvent,
      this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentIntensityNormalizationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveCursorMovingGUIEvents();
}

