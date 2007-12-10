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
#include "vtkKWPushButton.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkSlicerNodeSelectorWidget.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentRunSegmentationStep);
vtkCxxRevisionMacro(vtkEMSegmentRunSegmentationStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentRunSegmentationStep::vtkEMSegmentRunSegmentationStep()
{
  this->SetName("9/9. Run Segmentation");
  this->SetDescription("Apply EM algorithm to segment target image.");

  this->RunSegmentationSaveFrame                   = NULL;
  this->RunSegmentationDirectoryFrame              = NULL;
  this->RunSegmentationOutputFrame                 = NULL;
  this->RunSegmentationOutVolumeSelector           = NULL;
  this->RunSegmentationSaveTemplateButton          = NULL;
  this->RunSegmentationDirectorySubFrame           = NULL;
  this->RunSegmentationDirectoryButton             = NULL;
  this->RunSegmentationSaveAfterSegmentationCheckButton = NULL;
  this->RunSegmentationSaveIntermediateCheckButton = NULL;
  this->RunSegmentationGenerateSurfaceCheckButton  = NULL;
  this->RunSegmentationROIFrame                    = NULL;
  this->RunSegmentationROIMaxMatrix                = NULL;
  this->RunSegmentationROIMinMatrix                = NULL;
  this->RunSegmentationMiscFrame                   = NULL;
  this->RunSegmentationMultiThreadCheckButton      = NULL;
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
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

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
    this->RunSegmentationOutVolumeSelector->UpdateMenu();

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
      "Set the segmentation boundary max.");
    
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
  int ijk[3] = {0, 0, 0};
  mrmlManager->GetSegmentationBoundaryMax(ijk);
  this->PopulateSegmentationROIMatrix(maxMatrix, ijk);

  this->Script("pack %s -side left -anchor nw -padx 2 -pady 2",
               this->RunSegmentationROIMaxMatrix->GetWidgetName());
  this->RunSegmentationROIMaxMatrix->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

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
      "Set the segmentation boundary min.");
    
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
  mrmlManager->GetSegmentationBoundaryMin(ijk);
  this->PopulateSegmentationROIMatrix(minMatrix, ijk);

  this->Script("pack %s -side right -anchor nw -padx 2 -pady 2",
               this->RunSegmentationROIMinMatrix->GetWidgetName());
  this->RunSegmentationROIMinMatrix->SetEnabled(
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
void vtkEMSegmentRunSegmentationStep::PopulateSegmentationROIMatrix(
    vtkKWMatrixWidget* matrix, int ijk[3])
{
  if(matrix && ijk)
    {
    char buffer[10];
    for(int i=0; i<3; i++)
      {
      sprintf(buffer, "%d", ijk[i]);
      matrix->SetElementValue(0, i, buffer);
      }
    }
}

//---------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::AddRunRegistrationOutputGUIObservers() 
{
  this->RunSegmentationOutVolumeSelector->AddObserver(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RemoveRunRegistrationOutputGUIObservers()
{
  this->RunSegmentationOutVolumeSelector->RemoveObservers(
    vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
    this->GetGUI()->GetGUICallbackCommand());  
}

//---------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ProcessRunRegistrationOutputGUIEvents(
  vtkObject *caller,
  unsigned long event,
  void *callData) 
{
  if (caller == this->RunSegmentationOutVolumeSelector && 
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
      this->RunSegmentationOutVolumeSelector->GetSelected() != NULL) 
    { 
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    mrmlManager->SetOutputVolumeMRMLID(
      this->RunSegmentationOutVolumeSelector->GetSelected()->GetID());
    }
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
      mrmlManager->SetSaveTemplateFilename(filename.c_str());
      logic->SaveTemplateNow();
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SelectDirectoryCallback()
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
      mrmlManager->SetSaveWorkingDirectory(filename.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SaveAfterSegmentationCallback(
  int state)
{
  // The save template checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetSaveTemplateAfterSegmentation(state);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SaveIntermediateCallback(int state)
{
  // The save intermediate checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetSaveIntermediateResults(state);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::GenerateSurfaceModelsCallback(
  int state)
{
  // The save surface checkbutton has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->SetSaveSurfaceModels(state);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RunSegmentationROIMaxChangedCallback(
    int row, int col, const char *value)
{
  int ijk[3] = {0, 0, 0};
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  mrmlManager->GetSegmentationBoundaryMax(ijk);
  ijk[col] = atoi(value);
  if (mrmlManager->HasGlobalParametersNode())
    {
    mrmlManager->SetSegmentationBoundaryMax(ijk);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RunSegmentationROIMinChangedCallback(
    int row, int col, const char *value)
{
  int ijk[3] = {0, 0, 0};
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
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
  mrmlManager->SetEnableMultithreading(state);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::StartSegmentationCallback()
{
  vtkEMSegmentLogic *logic = this->GetGUI()->GetLogic();
  logic->StartSegmentation();
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::CancelSegmentationCallback()
{
  //vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  //mrmlManager->CancelSegmentation();
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
