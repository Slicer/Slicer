/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentRunSegmentationStep.cxx,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkEMSegmentRunSegmentationStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkImageData.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkLogLookupTable.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkWarpScalar.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentRunSegmentationStep);
vtkCxxRevisionMacro(vtkEMSegmentRunSegmentationStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentRunSegmentationStep::vtkEMSegmentRunSegmentationStep()
{
  this->SetName("9/9. Run Segmentation");
  this->SetDescription("Apply EM algorithm to segment target image.");

  this->RunSegmentationSaveFrame                        = NULL;
  this->RunSegmentationDirectoryFrame                   = NULL;
  this->RunSegmentationOutputFrame                      = NULL;
  this->RunSegmentationOutVolumeSelector                = NULL;
  this->RunSegmentationSaveTemplateButton               = NULL;
  this->RunSegmentationDirectorySubFrame                = NULL;
  this->RunSegmentationDirectoryButton                  = NULL;
  this->RunSegmentationSaveAfterSegmentationCheckButton = NULL;
  this->RunSegmentationSaveIntermediateCheckButton      = NULL;
  this->RunSegmentationGenerateSurfaceCheckButton       = NULL;
  this->RunSegmentationROIFrame                         = NULL;
  this->RunSegmentationROIMaxMatrix                     = NULL;
  this->RunSegmentationROIMinMatrix                     = NULL;
  this->RunSegmentationMiscFrame                        = NULL;
  this->RunSegmentationMultiThreadCheckButton           = NULL;
  this->RunRenderWidget                                 = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentRunSegmentationStep::~vtkEMSegmentRunSegmentationStep()
{
  if (this->RunSegmentationGenerateSurfaceCheckButton)
    {
    this->RunSegmentationGenerateSurfaceCheckButton->Delete();
    this->RunSegmentationGenerateSurfaceCheckButton = NULL;
    }

  if (this->RunSegmentationSaveIntermediateCheckButton)
    {
    this->RunSegmentationSaveIntermediateCheckButton->Delete();
    this->RunSegmentationSaveIntermediateCheckButton = NULL;
    }

  if (this->RunSegmentationSaveAfterSegmentationCheckButton)
    {
    this->RunSegmentationSaveAfterSegmentationCheckButton->Delete();
    this->RunSegmentationSaveAfterSegmentationCheckButton = NULL;
    }

  if (this->RunSegmentationDirectoryButton)
    {
    this->RunSegmentationDirectoryButton->Delete();
    this->RunSegmentationDirectoryButton = NULL;
    }

  if (this->RunSegmentationSaveTemplateButton)
    {
    this->RunSegmentationSaveTemplateButton->Delete();
    this->RunSegmentationSaveTemplateButton = NULL;
    }

  if (this->RunSegmentationDirectorySubFrame)
    {
    this->RunSegmentationDirectorySubFrame->Delete();
    this->RunSegmentationDirectorySubFrame = NULL;
    }

  if (this->RunSegmentationSaveFrame)
    {
    this->RunSegmentationSaveFrame->Delete();
    this->RunSegmentationSaveFrame = NULL;
    }

  if (this->RunSegmentationDirectoryFrame)
    {
    this->RunSegmentationDirectoryFrame->Delete();
    this->RunSegmentationDirectoryFrame = NULL;
    }

  if (this->RunSegmentationOutVolumeSelector)
    {
    this->RunSegmentationOutVolumeSelector->Delete();
    this->RunSegmentationOutVolumeSelector = NULL;
    }

  if (this->RunSegmentationOutputFrame)
    {
    this->RunSegmentationOutputFrame->Delete();
    this->RunSegmentationOutputFrame = NULL;
    }

  if (this->RunSegmentationROIMaxMatrix)
    {
    this->RunSegmentationROIMaxMatrix->Delete();
    this->RunSegmentationROIMaxMatrix = NULL;
    }

  if (this->RunSegmentationROIMinMatrix)
    {
    this->RunSegmentationROIMinMatrix->Delete();
    this->RunSegmentationROIMinMatrix = NULL;
    }

  if (this->RunSegmentationROIFrame)
    {
    this->RunSegmentationROIFrame->Delete();
    this->RunSegmentationROIFrame = NULL;
    }

  if (this->RunSegmentationMultiThreadCheckButton)
    {
    this->RunSegmentationMultiThreadCheckButton->Delete();
    this->RunSegmentationMultiThreadCheckButton = NULL;
    }

  if (this->RunSegmentationMiscFrame)
    {
    this->RunSegmentationMiscFrame->Delete();
    this->RunSegmentationMiscFrame = NULL;
    }

  if (this->RunRenderWidget)
  {
    this->RunRenderWidget->Delete();
    this->RunRenderWidget = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentMRMLManager *mrmlManager =
    this->GetGUI()->GetMRMLManager();

  vtkKWWizardWidget *wizard_widget =
    this->GetGUI()->GetWizardWidget();

  if (!mrmlManager || !wizard_widget)
  {
    return;
  }

  vtkKWWidget *parent = wizard_widget->GetClientArea();;

  int enabled = parent->GetEnabled();

  wizard_widget->GetCancelButton()->SetEnabled(enabled);

  // Create the save frame

  if (!this->RunSegmentationSaveFrame)
  {
    this->RunSegmentationSaveFrame = vtkKWFrameWithLabel::New();
  }

  if (!this->RunSegmentationSaveFrame->IsCreated())
  {
    this->RunSegmentationSaveFrame->SetParent(parent);
    this->RunSegmentationSaveFrame->Create();
    this->RunSegmentationSaveFrame->SetLabelText("Save");
  }

  // disable this frame for now, it is not currently useful
  //   this->Script(
  //     "pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
  //     this->RunSegmentationSaveFrame->GetWidgetName());

  // Create the "save after segmentation" checkbutton

  if (!this->RunSegmentationSaveAfterSegmentationCheckButton)
    {
    this->RunSegmentationSaveAfterSegmentationCheckButton =
      vtkKWCheckButtonWithLabel::New();
    }

  if (!this->RunSegmentationSaveAfterSegmentationCheckButton->IsCreated())
    {
    this->RunSegmentationSaveAfterSegmentationCheckButton->SetParent(
      this->RunSegmentationSaveFrame->GetFrame());
    this->RunSegmentationSaveAfterSegmentationCheckButton->Create();
    this->RunSegmentationSaveAfterSegmentationCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 6);
    this->RunSegmentationSaveAfterSegmentationCheckButton->GetLabel()->
      SetText("Save After Segmentation:");
    this->RunSegmentationSaveAfterSegmentationCheckButton->
      GetWidget()->SetCommand(this, "SaveAfterSegmentationCallback");
    }
  this->RunSegmentationSaveAfterSegmentationCheckButton->
    GetWidget()->
    SetSelectedState(mrmlManager->GetSaveTemplateAfterSegmentation());

  this->Script(
    "pack %s -side left -anchor nw -padx 2 -pady 2", 
    this->RunSegmentationSaveAfterSegmentationCheckButton->GetWidgetName());

  this->RunSegmentationSaveAfterSegmentationCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create the save template button

  if (!this->RunSegmentationSaveTemplateButton)
    {
    this->RunSegmentationSaveTemplateButton = 
      vtkKWLoadSaveButton::New();
    }
  if (!this->RunSegmentationSaveTemplateButton->IsCreated())
    {
    this->RunSegmentationSaveTemplateButton->SetParent(
      this->RunSegmentationSaveFrame->GetFrame());
    this->RunSegmentationSaveTemplateButton->Create();
    this->RunSegmentationSaveTemplateButton->SetImageToPredefinedIcon(
      vtkKWIcon::IconFloppy);
    this->RunSegmentationSaveTemplateButton->TrimPathFromFileNameOn();
    this->RunSegmentationSaveTemplateButton->SetBalloonHelpString(
      "Select a file for saving the template");
    this->RunSegmentationSaveTemplateButton->SetCommand(
      this, "SelectTemplateFileCallback");
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->SaveDialogOn();
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
      SetFileTypes("{{MRML File} {.mrml}} {{All Files} {*.*}}");
    }

  if (mrmlManager->GetSaveTemplateFilename() &&
      vtksys::SystemTools::FileExists(mrmlManager->GetSaveTemplateFilename()))
    {
    vtksys_stl::string filename = mrmlManager->GetSaveTemplateFilename();
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
      GenerateLastPath(filename.c_str());
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
      SetInitialFileName(filename.c_str());
    }
  else
    {
    this->RunSegmentationSaveTemplateButton->SetText("Save Template File");
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
      RetrieveLastPathFromRegistry("OpenPath");
    }
  this->RunSegmentationSaveTemplateButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side left -anchor nw -fill x -padx 2 -pady 2", 
    this->RunSegmentationSaveTemplateButton->GetWidgetName());

  // Create the working directory frame

  if (!this->RunSegmentationDirectoryFrame)
    {
    this->RunSegmentationDirectoryFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RunSegmentationDirectoryFrame->IsCreated())
    {
    this->RunSegmentationDirectoryFrame->SetParent(parent);
    this->RunSegmentationDirectoryFrame->Create();
    this->RunSegmentationDirectoryFrame->SetLabelText("Working Directory");
    }
  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->RunSegmentationDirectoryFrame->GetWidgetName());

  // Create the frame for the directory sub frame

  if (!this->RunSegmentationDirectorySubFrame)
    {
    this->RunSegmentationDirectorySubFrame = vtkKWFrame::New();
    }
  if (!this->RunSegmentationDirectorySubFrame->IsCreated())
    {
    this->RunSegmentationDirectorySubFrame->SetParent(
      this->RunSegmentationDirectoryFrame->GetFrame());
    this->RunSegmentationDirectorySubFrame->Create();
    }

  this->Script(
    "pack %s -side left -anchor nw -fill x -padx 0 -pady 0", 
    this->RunSegmentationDirectorySubFrame->GetWidgetName());

  // Create the save intermediate results button

  if (!this->RunSegmentationSaveIntermediateCheckButton)
    {
    this->RunSegmentationSaveIntermediateCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->RunSegmentationSaveIntermediateCheckButton->IsCreated())
    {
    this->RunSegmentationSaveIntermediateCheckButton->SetParent(
      this->RunSegmentationDirectorySubFrame);
    this->RunSegmentationSaveIntermediateCheckButton->Create();
    this->RunSegmentationSaveIntermediateCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 6);
    this->RunSegmentationSaveIntermediateCheckButton->GetLabel()->
      SetText("Save Intermediate Results:");
    this->RunSegmentationSaveIntermediateCheckButton->
      GetWidget()->SetCommand(this, "SaveIntermediateCallback");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RunSegmentationSaveIntermediateCheckButton->GetWidgetName());

  this->RunSegmentationSaveIntermediateCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetSaveIntermediateResults());
  this->RunSegmentationSaveIntermediateCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create the generate surface model button

  if (!this->RunSegmentationGenerateSurfaceCheckButton)
    {
    this->RunSegmentationGenerateSurfaceCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->RunSegmentationGenerateSurfaceCheckButton->IsCreated())
    {
    this->RunSegmentationGenerateSurfaceCheckButton->SetParent(
      this->RunSegmentationDirectorySubFrame);
    this->RunSegmentationGenerateSurfaceCheckButton->Create();
    this->RunSegmentationGenerateSurfaceCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 6);
    this->RunSegmentationGenerateSurfaceCheckButton->
      SetLabelText("Generate Surface Model:");
    this->RunSegmentationGenerateSurfaceCheckButton->
      GetWidget()->SetCommand(this, "GenerateSurfaceModelsCallback");
    }

  // disable for now, not used and confusing
  //   this->Script(
  //     "pack %s -side top -anchor nw -padx 2 -pady 2", 
  //     this->RunSegmentationGenerateSurfaceCheckButton->GetWidgetName());
  
  this->RunSegmentationGenerateSurfaceCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetSaveSurfaceModels());
  this->RunSegmentationGenerateSurfaceCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create the directory button

  if (!this->RunSegmentationDirectoryButton)
    {
    this->RunSegmentationDirectoryButton = vtkKWLoadSaveButton::New();
    }
  if (!this->RunSegmentationDirectoryButton->IsCreated())
    {
    this->RunSegmentationDirectoryButton->SetParent(
      this->RunSegmentationDirectoryFrame->GetFrame());
    this->RunSegmentationDirectoryButton->Create();
    this->RunSegmentationDirectoryButton->SetImageToPredefinedIcon(
      vtkKWIcon::IconFolderOpen);
    this->RunSegmentationDirectoryButton->TrimPathFromFileNameOn();
    this->RunSegmentationDirectoryButton->SetBalloonHelpString(
      "Select the working directory");
    this->RunSegmentationDirectoryButton->SetCommand(
      this, "SelectDirectoryCallback");
    this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->ChooseDirectoryOn();
    }

  if (mrmlManager->GetSaveWorkingDirectory() &&
    vtksys::SystemTools::FileIsDirectory(mrmlManager->GetSaveWorkingDirectory()))
    {
    vtksys_stl::string path = mrmlManager->GetSaveWorkingDirectory();
    this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
      GenerateLastPath(path.c_str());
    this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
      SetInitialFileName(path.c_str());
    }
  else
    {
    this->RunSegmentationDirectoryButton->SetText("Select Working Directory");
    this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
      RetrieveLastPathFromRegistry("OpenPath");
    }

  this->RunSegmentationDirectoryButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side left -anchor nw -fill x -padx 2 -pady 2", 
    this->RunSegmentationDirectoryButton->GetWidgetName());
  
  // Create the output image frame

  if (!this->RunSegmentationOutputFrame)
    {
    this->RunSegmentationOutputFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RunSegmentationOutputFrame->IsCreated())
    {
    this->RunSegmentationOutputFrame->SetParent(parent);
    this->RunSegmentationOutputFrame->Create();
    this->RunSegmentationOutputFrame->SetLabelText("Output Labelmap");
    }
  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->RunSegmentationOutputFrame->GetWidgetName());
  
  // Create the output image frame

  if (!this->RunSegmentationOutVolumeSelector)
    {
    this->RunSegmentationOutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->RunSegmentationOutVolumeSelector->IsCreated())
    {
    this->RunSegmentationOutVolumeSelector->SetNodeClass(
      "vtkMRMLScalarVolumeNode", 
      "LabelMap", "1", 
      "EM Segmentation");
    this->RunSegmentationOutVolumeSelector->SetNewNodeEnabled(1);
    this->RunSegmentationOutVolumeSelector->SetParent(
      this->RunSegmentationOutputFrame->GetFrame());
    this->RunSegmentationOutVolumeSelector->Create();
    this->RunSegmentationOutVolumeSelector->
      SetMRMLScene(mrmlManager->GetMRMLScene());

    this->RunSegmentationOutVolumeSelector->SetBorderWidth(2);
    this->RunSegmentationOutVolumeSelector->SetLabelText( "Output Labelmap: ");
    this->RunSegmentationOutVolumeSelector->SetBalloonHelpString(
      "select an output labelmap from the current mrml scene.");
    }
  this->RunSegmentationOutVolumeSelector->UpdateMenu();
  if(mrmlManager->GetOutputVolumeMRMLID())
    {
    this->RunSegmentationOutVolumeSelector->SetSelected(
      this->RunSegmentationOutVolumeSelector->GetMRMLScene()->
      GetNodeByID(mrmlManager->GetOutputVolumeMRMLID()));
    }

  this->RunSegmentationOutVolumeSelector->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RunSegmentationOutVolumeSelector->GetWidgetName());
  
  this->AddRunRegistrationOutputGUIObservers();
  
  // Create the boundary frame

  if (!this->RunSegmentationROIFrame)
    {
      this->RunSegmentationROIFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RunSegmentationROIFrame->IsCreated())
    {
    this->RunSegmentationROIFrame->SetParent(parent);
    this->RunSegmentationROIFrame->Create();
    this->RunSegmentationROIFrame->SetLabelText("ROI");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->RunSegmentationROIFrame->GetWidgetName());

  // Create Min boundary matrix

  if (!this->RunSegmentationROIMinMatrix)
    {
    this->RunSegmentationROIMinMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->RunSegmentationROIMinMatrix->IsCreated())
    {
    this->RunSegmentationROIMinMatrix->SetParent(
      this->RunSegmentationROIFrame->GetFrame());
    this->RunSegmentationROIMinMatrix->Create();
    this->RunSegmentationROIMinMatrix->SetLabelText("Min (i,j,k):");
    this->RunSegmentationROIMinMatrix->SetLabelPositionToLeft();
    this->RunSegmentationROIMinMatrix->ExpandWidgetOff();
    this->RunSegmentationROIMinMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 18);
    this->RunSegmentationROIMinMatrix->SetBalloonHelpString(
      "Set the segmentation boundary min.  By default the entire image is used.");
    
    vtkKWMatrixWidget *matrix = 
      this->RunSegmentationROIMinMatrix->GetWidget();
    matrix->SetNumberOfColumns(3);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(4);
    matrix->SetRestrictElementValueToInteger();
    matrix->SetElementChangedCommand(
      this, "RunSegmentationROIMinChangedCallback");
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }
  vtkKWMatrixWidget *minMatrix = 
    this->RunSegmentationROIMinMatrix->GetWidget();
  int ijk[3] = {0, 0, 0};
  mrmlManager->GetSegmentationBoundaryMin(ijk);
  this->PopulateSegmentationROIMatrix(minMatrix, ijk);

  this->Script("pack %s -side left -anchor nw -padx 2 -pady 2",
               this->RunSegmentationROIMinMatrix->GetWidgetName());
  this->RunSegmentationROIMinMatrix->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create Max boundary matrix

  if (!this->RunSegmentationROIMaxMatrix)
    {
    this->RunSegmentationROIMaxMatrix = 
      vtkKWMatrixWidgetWithLabel::New();
    }
  if (!this->RunSegmentationROIMaxMatrix->IsCreated())
    {
    this->RunSegmentationROIMaxMatrix->SetParent(
      this->RunSegmentationROIFrame->GetFrame());
    this->RunSegmentationROIMaxMatrix->Create();
    this->RunSegmentationROIMaxMatrix->SetLabelText("Max (i,j,k):");
    this->RunSegmentationROIMaxMatrix->SetLabelPositionToLeft();
    this->RunSegmentationROIMaxMatrix->ExpandWidgetOff();
    this->RunSegmentationROIMaxMatrix->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 16);
    this->RunSegmentationROIMaxMatrix->SetBalloonHelpString(
      "Set the segmentation boundary max.  By default the entire image is used.");
    
    vtkKWMatrixWidget *matrix = 
      this->RunSegmentationROIMaxMatrix->GetWidget();
    matrix->SetNumberOfColumns(3);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(4);
    matrix->SetRestrictElementValueToInteger();
    matrix->SetElementChangedCommand(
      this, "RunSegmentationROIMaxChangedCallback");
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }
  vtkKWMatrixWidget *maxMatrix = 
    this->RunSegmentationROIMaxMatrix->GetWidget();
  mrmlManager->GetSegmentationBoundaryMax(ijk);
  this->PopulateSegmentationROIMatrix(maxMatrix, ijk);

  this->Script("pack %s -side right -anchor nw -padx 2 -pady 2",
               this->RunSegmentationROIMaxMatrix->GetWidgetName());
  this->RunSegmentationROIMaxMatrix->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create the run frame

  if (!this->RunSegmentationMiscFrame)
    {
    this->RunSegmentationMiscFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RunSegmentationMiscFrame->IsCreated())
    {
    this->RunSegmentationMiscFrame->SetParent(parent);
    this->RunSegmentationMiscFrame->Create();
    this->RunSegmentationMiscFrame->SetLabelText("Misc.");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
    this->RunSegmentationMiscFrame->GetWidgetName());

  // Create the multithread button

  if (!this->RunSegmentationMultiThreadCheckButton)
    {
    this->RunSegmentationMultiThreadCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->RunSegmentationMultiThreadCheckButton->IsCreated())
    {
    this->RunSegmentationMultiThreadCheckButton->SetParent(
      this->RunSegmentationMiscFrame->GetFrame());
    this->RunSegmentationMultiThreadCheckButton->Create();
    this->RunSegmentationMultiThreadCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RunSegmentationMultiThreadCheckButton->
      SetLabelText("Multi-threading Enabled:");
    this->RunSegmentationMultiThreadCheckButton->
      GetWidget()->SetCommand(this, "MultiThreadingCallback");
    }

  this->RunSegmentationMultiThreadCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2",
    this->RunSegmentationMultiThreadCheckButton->GetWidgetName());

  this->RunSegmentationMultiThreadCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetEnableMultithreading());

  int windowWidth  = 400;
  int windowHeight = 400;

  if (!this->RunRenderWidget)
  {
    this->RunRenderWidget = vtkKWRenderWidget::New();
  }

  if (!this->RunRenderWidget->IsCreated())
  {
    this->RunRenderWidget->SetParent(
      this->RunSegmentationMiscFrame->GetFrame());
    this->RunRenderWidget->Create();
    //this->Gaussian2DWidget->GetRenderer()->GetRenderWindow()->
    //GetInteractor()->Disable();
    this->RunRenderWidget->SetWidth(windowWidth);
    this->RunRenderWidget->SetHeight(windowHeight);
    this->RunRenderWidget->Render();
  }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2",
    this->RunRenderWidget->GetWidgetName());

  // Configure the OK button to start

  if (wizard_widget->GetOKButton())
    {
    wizard_widget->GetOKButton()->SetText("Run");
    wizard_widget->GetOKButton()->SetCommand(
      this, "StartSegmentationCallback");
    wizard_widget->GetOKButton()->SetBalloonHelpString(
      "Start Segmentation");
    }

  // Configure the cancel button

  if (wizard_widget->GetCancelButton())
    {
    wizard_widget->GetCancelButton()->SetCommand(
      this, "CancelSegmentationCallback");
    wizard_widget->GetCancelButton()->SetBalloonHelpString(
      "Cancel Segmentation");
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::HideUserInterface()
{
  this->Superclass::HideUserInterface();
  this->RemoveRunRegistrationOutputGUIObservers();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentRunSegmentationStep::
PopulateSegmentationROIMatrix(
    vtkKWMatrixWidget* matrix, int ijk[3])
{
  if (matrix && ijk)
  {
    char buffer[10];

    sprintf(buffer,"%d",ijk[0]);
    matrix->SetElementValue(0,0,buffer);

    sprintf(buffer,"%d",ijk[1]);
    matrix->SetElementValue(0,1,buffer);

    sprintf(buffer,"%d",ijk[2]);
    matrix->SetElementValue(0,2,buffer);
  }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentRunSegmentationStep::
AddRunRegistrationOutputGUIObservers()
{
  this->RunSegmentationOutVolumeSelector->AddObserver(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
    this->GetGUI()->GetGUICallbackCommand());
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::
RemoveRunRegistrationOutputGUIObservers()
{
  this->RunSegmentationOutVolumeSelector->RemoveObservers(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
    this->GetGUI()->GetGUICallbackCommand());
}

//---------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ProcessRunRegistrationOutputGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *vtkNotUsed(callData))
{
  if (caller == this->RunSegmentationOutVolumeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
      this->RunSegmentationOutVolumeSelector->GetSelected() != NULL)
  {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

    if (mrmlManager)
      {
      mrmlManager->SetOutputVolumeMRMLID(
        this->RunSegmentationOutVolumeSelector->GetSelected()->GetID());
      }
  }
  /*
  else if (this->RunRenderWidget)
  {
    if (!this->RunRenderWidget->GetRenderer()->GetNumberOfPropsRendered())
    {
      vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

      if (!mrmlManager)
      {
        vtkErrorMacro("No MRML Manager");
        return;
      }

      int numTargets = mrmlManager->GetTargetNumberOfSelectedVolumes();

      if (numTargets < 2)
      {
        vtkErrorMacro("Two target images are necessary. Number of Targets: "
            << numTargets);
        return;
      }

      int volumeId = mrmlManager->GetVolumeNthID(0);

      vtkMRMLVolumeNode* volumeNode = mrmlManager->GetVolumeNode(volumeId);

      if(!volumeNode)
      {
        vtkErrorMacro("No target volume 1");
        return;
      }

      vtkImageData* inputImage1 = volumeNode->GetImageData();

      if (!inputImage1)
      {
        vtkErrorMacro("Target 1 has no image data");
        return;
      }

      volumeId = mrmlManager->GetVolumeNthID(1);

      volumeNode = mrmlManager->GetVolumeNode(volumeId);

      if(!volumeNode)
      {
        vtkErrorMacro("No target volume 2");
        return;
      }

      vtkImageData* inputImage2 = volumeNode->GetImageData();

      if (!inputImage2)
      {
        vtkErrorMacro("Target 2 has no image data");
        return;
      }

      if (inputImage1->GetNumberOfScalarComponents() != 1)
      {
        vtkErrorMacro("Voxels in Target 1 must have 1 scalar components. "
            << "Number of scalar components: "
            << inputImage1->GetNumberOfScalarComponents());
        return;
      }

      if (inputImage2->GetNumberOfScalarComponents() != 1)
      {
        vtkErrorMacro("Voxels in Target 2 must have 1 scalar components. "
            << "Number of scalar components: "
            << inputImage2->GetNumberOfScalarComponents());
        return;
      }

      if (inputImage1->GetScalarType() != VTK_SHORT)
      {
        vtkErrorMacro("Scalar type in Target 1 must be SHORT. "
            << "Scalar type: "
            << inputImage1->GetScalarType());
        return;
      }

      if (inputImage2->GetScalarType() != VTK_SHORT)
      {
        vtkErrorMacro("Scalar type in Target 2 must be SHORT. "
            << "Scalar type: "
            << inputImage2->GetScalarType());
        return;
      }

      int extent1[6];
      int extent2[6];

      inputImage1->GetWholeExtent(extent1);
      inputImage2->GetWholeExtent(extent2);

      if( extent1[0] != extent2[0] ||
          extent1[1] != extent2[1] ||
          extent1[2] != extent2[2] ||
          extent1[3] != extent2[3] ||
          extent1[4] != extent2[4] ||
          extent1[5] != extent2[5] )
      {
        vtkErrorMacro("Target 1 and Target 2 have different extent. "
            << "Target 1 whole extent: "
            << extent1[0] << " " << extent1[1] << " "
            << extent1[2] << " " << extent1[3] << " "
            << extent1[4] << " " << extent1[5] << " "
            << "Target 2 whole extent: "
            << extent2[0] << " " << extent2[1] << " "
            << extent2[2] << " " << extent2[3] << " "
            << extent2[4] << " " << extent2[5]);
        return;
      }

      int numPts1 = inputImage1->GetNumberOfPoints();
      int numPts2 = inputImage2->GetNumberOfPoints();

      if(numPts1 != numPts2)
      {
        vtkErrorMacro("Target Image 1 and Target Image 2 have a different"
            << " number of voxels."
            << " Target 1 NumVoxels: " << numPts1
            << " Target 2 NumVoxels: " << numPts2);
        return;
      }

      double range1[2];
      double range2[2];

      inputImage1->GetScalarRange(range1);
      inputImage2->GetScalarRange(range2);

      if(range1[1] <= range1[0])
      {
        vtkErrorMacro("Target 1 has invalid scalar range. "
            << "Scalar Range " << range1[0] << " " << range1[1]);
        return;
      }

      if(range2[1] <= range2[0])
      {
        vtkErrorMacro("Target 2 has invalid scalar range. "
            << "Scalar range: " << range2[0] << " " << range2[1]);
        return;
      }

      int dimX = 100;
      int dimY = 100;

      int extent[6] = {0,0, 0,0, 0,0};

      extent[1] = dimX - 1;
      extent[3] = dimY - 1;

      double factorX = dimX / (range1[1] - range1[0]);
      double factorY = dimY / (range2[1] - range2[0]);

      vtkImageData *imageData = vtkImageData::New();
      imageData->SetScalarTypeToUnsignedInt();
      imageData->SetWholeExtent(extent);
      imageData->AllocateScalars();
      imageData->Update();

      int numPts = imageData->GetNumberOfPoints();

      unsigned int *ptr = (unsigned int*) imageData->GetScalarPointer();

      memset(ptr,0,numPts*sizeof(unsigned int));

      short *ptr1 = (short*) inputImage1->GetScalarPointer();
      short *ptr2 = (short*) inputImage2->GetScalarPointer();

      for(int i=0; i<numPts1; i++,ptr1++,ptr2++)
      {
        int id1 = (int)((*ptr1 - range1[0]) * factorX);
        int id2 = (int)((*ptr2 - range2[0]) * factorY);

        if(id1 > extent[1]) { id1--; }
        if(id2 > extent[3]) { id2--; }

        if(id1 < 0) { id1=0; }
        if(id2 < 0) { id2=0; }

        int id = id1 + dimX * id2;
        if(id) { ptr[id]++; }
      }

      ptr = (unsigned int*) imageData->GetScalarPointer();

      for(int i=0; i<numPts; i++,ptr++)
      {
        if(*ptr) { *ptr = static_cast<unsigned int> (log(static_cast<float>(*ptr))); }
      }

      double range[2];
      imageData->GetScalarRange(range);

      vtkImageDataGeometryFilter *geometryFilter =
        vtkImageDataGeometryFilter::New();
      geometryFilter->SetInput(imageData);
      imageData->Delete();

      vtkWarpScalar *warpScalar = vtkWarpScalar::New();
      warpScalar->SetInput(geometryFilter->GetOutput());
      warpScalar->Update();

      vtkPolyData *polyData = vtkPolyData::New();
      polyData->DeepCopy(geometryFilter->GetOutput());
      geometryFilter->Delete();
      polyData->SetPoints(warpScalar->GetOutput()->GetPoints());
      warpScalar->Delete();

      vtkLogLookupTable *LUT = vtkLogLookupTable::New();
      LUT->SetTableRange(range);
      LUT->SetHueRange(0.667,0);
      LUT->SetSaturationRange(1,1);
      LUT->SetValueRange(1,1);

      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetScalarRange(range);
      mapper->SetInput(polyData);
      polyData->Delete();
      mapper->SetLookupTable(LUT);
      LUT->Delete();

      vtkActor *actor = vtkActor::New();
      actor->SetMapper(mapper);
      mapper->Delete();

      this->RunRenderWidget->AddViewProp(actor);
      actor->Delete();
    }
  }
*/
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SelectTemplateFileCallback()
{
  // The template file has changed because of user interaction

  if (this->RunSegmentationSaveTemplateButton &&
      this->RunSegmentationSaveTemplateButton->IsCreated())
    {
    if (this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
        GetStatus() == vtkKWDialog::StatusOK)
      {
      this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
        SaveLastPathToRegistry("OpenPath");
      vtksys_stl::string filename =
        this->RunSegmentationSaveTemplateButton->GetFileName();
      vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
      vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
      if (mrmlManager)
        {
        mrmlManager->SetSaveTemplateFilename(filename.c_str());
        }
      if (logic)
        {
        logic->SaveTemplateNow();
        }
      }
    }
}

//----------------------------------------------------------------------------
void
vtkEMSegmentRunSegmentationStep::
SelectDirectoryCallback()
{
  // The template file has changed because of user interaction

  if (this->RunSegmentationDirectoryButton &&
      this->RunSegmentationDirectoryButton->IsCreated())
    {
    if (this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
        GetStatus() == vtkKWDialog::StatusOK)
      {
      this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
        SaveLastPathToRegistry("OpenPath");

      vtksys_stl::string filename =
        this->RunSegmentationDirectoryButton->GetFileName();

      if(!vtksys::SystemTools::FileExists(filename.c_str()) ||
        !vtksys::SystemTools::FileIsDirectory(filename.c_str()))
        {
        if(!vtksys::SystemTools::MakeDirectory(filename.c_str()))
          {
          vtkErrorMacro("Can not create directory: " << filename.c_str());
          return;
          }
        }

      vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
      if (mrmlManager)
        {
        mrmlManager->SetSaveWorkingDirectory(filename.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SaveAfterSegmentationCallback(
  int state)
{
  // The save template checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetSaveTemplateAfterSegmentation(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SaveIntermediateCallback(int state)
{
  // The save intermediate checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetSaveIntermediateResults(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::GenerateSurfaceModelsCallback(
  int state)
{
  // The save surface checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetSaveSurfaceModels(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RunSegmentationROIMaxChangedCallback(
    int vtkNotUsed(row), int col, const char *value)
{
  int ijk[3] = {0, 0, 0};
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->GetSegmentationBoundaryMax(ijk);
  ijk[col] = atoi(value);
  if (mrmlManager->HasGlobalParametersNode())
    {
    mrmlManager->SetSegmentationBoundaryMax(ijk);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RunSegmentationROIMinChangedCallback(
    int vtkNotUsed(row), int col, const char *value)
{
  int ijk[3] = {0, 0, 0};
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->GetSegmentationBoundaryMin(ijk);
  ijk[col] = atoi(value);
  if (mrmlManager->HasGlobalParametersNode())
    {
    mrmlManager->SetSegmentationBoundaryMin(ijk);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::MultiThreadingCallback(int state)
{
  // The multithreading checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetEnableMultithreading(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::StartSegmentationCallback()
{
  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  vtkEMSegmentMRMLManager* mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager || !logic)
    {
    return;
    }
  
  // make sure that data types are the same
  if (!mrmlManager->DoTargetAndAtlasDataTypesMatch())
    {
    // popup an error message
    std::string errorMessage = 
      "Scalar type mismatch for input images; all image scalar types must be "
      "the same (including input channels and atlas images).";

    vtkKWMessageDialog::PopupMessage(this->GetApplication(),
                                     NULL,
                                     "Input Image Error",
                                     errorMessage.c_str(),
                                     vtkKWMessageDialog::ErrorIcon | 
                                     vtkKWMessageDialog::InvokeAtPointer);    
    return;
    }

  // start the segmentation
  logic->StartSegmentation();
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::CancelSegmentationCallback()
{
  //vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  //if (mrmlManager)
  //  {
  //    mrmlManager->CancelSegmentation();
  //  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
