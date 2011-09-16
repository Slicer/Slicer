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
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"

#include <vtksys/SystemTools.hxx>
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkKWLabel.h"
#include "vtkKWRange.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkImageRectangularSource.h"
#include "vtkMRMLROINode.h"
#include "vtkSlicerROIDisplayWidget.h"
#include "vtkObserverManager.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkKWScale.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWProgressDialog.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentRunSegmentationStep);
vtkCxxRevisionMacro(vtkEMSegmentRunSegmentationStep, "$Revision: 1.2 $");


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


//----------------------------------------------------------------------------
vtkEMSegmentRunSegmentationStep::vtkEMSegmentRunSegmentationStep()
{
  this->SetName("9/9. Define Miscellaneous Parameters ");
  this->SetDescription("Define miscellaneous parameter for performing segmentation.");

  this->RunSegmentationSaveFrame                   = NULL;
  this->RunSegmentationDirectoryFrame              = NULL;
  //  this->RunSegmentationOutputFrame                 = NULL;
  //this->RunSegmentationOutVolumeSelector           = NULL;
  this->RunSegmentationSaveTemplateFrame = NULL;
  this->RunSegmentationSaveTemplateButton          = NULL;
  this->RunSegmentationDirectorySubFrame           = NULL;
this->RunSegmentationDirectoryLabel              = NULL;
  this->RunSegmentationDirectoryButton             = NULL;
  this->RunSegmentationSaveTemplateLabel = NULL;
  this->RunSegmentationSaveIntermediateCheckButton = NULL;
  this->RunSegmentationGenerateSurfaceCheckButton  = NULL;
  this->RunSegmentationMiscFrame                   = NULL;
  this->RunSegmentationMultiThreadCheckButton      = NULL;
  this->PostprocessingFrame = NULL;
  this->PostprocessingMinimumIslandSize = NULL;
  this->PostprocessingIsland2DCheckButton = NULL;
  this->PostprocessingSubparcellationCheckButton = NULL;

  this->InitialROIWidget() ;
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
  if (this->RunSegmentationSaveTemplateFrame)
    {
    this->RunSegmentationSaveTemplateFrame->Delete();
    this->RunSegmentationSaveTemplateFrame = NULL;
    }


  if (this->RunSegmentationSaveTemplateLabel)
    {
    this->RunSegmentationSaveTemplateLabel->Delete();
    this->RunSegmentationSaveTemplateLabel = NULL;
    }

  if (this->RunSegmentationDirectoryLabel)
    {
     this->RunSegmentationDirectoryLabel->Delete();
     this->RunSegmentationDirectoryLabel = NULL;
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

//   if (this->RunSegmentationOutVolumeSelector)
//     {
//     this->RunSegmentationOutVolumeSelector->Delete();
//     this->RunSegmentationOutVolumeSelector = NULL;
//     }
// 
//   if (this->RunSegmentationOutputFrame)
//     {
//     this->RunSegmentationOutputFrame->Delete();
//     this->RunSegmentationOutputFrame = NULL;
//     }


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

  if (this->PostprocessingFrame)
    {
      this->PostprocessingFrame->Delete();
      this->PostprocessingFrame = NULL;
    } 

  if (this->PostprocessingMinimumIslandSize)
    {
      this->PostprocessingMinimumIslandSize->Delete();
       this->PostprocessingMinimumIslandSize = NULL;
    } 
 
  if (this->PostprocessingIsland2DCheckButton)
    {
      this->PostprocessingIsland2DCheckButton->Delete();
      this->PostprocessingIsland2DCheckButton = NULL;
    }

  if (this->PostprocessingSubparcellationCheckButton)
    {
       this->PostprocessingSubparcellationCheckButton->Delete();
       this->PostprocessingSubparcellationCheckButton = NULL;
    } 

  this->ResetROIWidget() ;
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }

  vtkKWWidget *parent = wizard_widget->GetClientArea();;
  int enabled = parent->GetEnabled();
  wizard_widget->GetCancelButton()->SetEnabled(enabled);

  // Create the boundary frame

  this->ShowROIGUI(parent); 


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
   this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->RunSegmentationSaveFrame->GetWidgetName());
  

 if (!this->RunSegmentationSaveTemplateFrame)
    {
    this->RunSegmentationSaveTemplateFrame = vtkKWFrame::New();
    }
  if (!this->RunSegmentationSaveTemplateFrame->IsCreated())
    {
      this->RunSegmentationSaveTemplateFrame->SetParent(this->RunSegmentationSaveFrame->GetFrame());
    this->RunSegmentationSaveTemplateFrame->Create();
    }
   this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->RunSegmentationSaveTemplateFrame->GetWidgetName());


  // Create the "save after segmentation" checkbutton

  if (!this->RunSegmentationSaveTemplateLabel)
    {
    this->RunSegmentationSaveTemplateLabel = vtkKWLabel::New();
    }

  if (!this->RunSegmentationSaveTemplateLabel->IsCreated())
    {
    this->RunSegmentationSaveTemplateLabel->SetParent(this->RunSegmentationSaveTemplateFrame);
    this->RunSegmentationSaveTemplateLabel->Create();
    this->RunSegmentationSaveTemplateLabel->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RunSegmentationSaveTemplateLabel->SetText("Create Template File:");
    }
  this->RunSegmentationSaveTemplateLabel->SetEnabled( mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  // Create the save template button

  if (!this->RunSegmentationSaveTemplateButton)
    {
    this->RunSegmentationSaveTemplateButton = 
      vtkKWLoadSaveButton::New();
    }
  if (!this->RunSegmentationSaveTemplateButton->IsCreated())
    {
      this->RunSegmentationSaveTemplateButton->SetParent(this->RunSegmentationSaveTemplateFrame);
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
    this->RunSegmentationSaveTemplateButton->SetText("Create");
    this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->
      RetrieveLastPathFromRegistry("OpenPath");
    }
  this->RunSegmentationSaveTemplateButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script( "pack %s %s -side left -anchor w -padx 2 -pady 2", this->RunSegmentationSaveTemplateLabel->GetWidgetName() , this->RunSegmentationSaveTemplateButton->GetWidgetName());


  // Intermediate results 
 if (!this->RunSegmentationSaveIntermediateCheckButton)
    {
    this->RunSegmentationSaveIntermediateCheckButton = 
      vtkKWCheckButtonWithLabel::New();
    }
  if (!this->RunSegmentationSaveIntermediateCheckButton->IsCreated())
    {
      this->RunSegmentationSaveIntermediateCheckButton->SetParent( this->RunSegmentationSaveFrame->GetFrame());
    this->RunSegmentationSaveIntermediateCheckButton->Create();
    this->RunSegmentationSaveIntermediateCheckButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RunSegmentationSaveIntermediateCheckButton->GetLabel()->
      SetText("Save Intermediate Results: ");
    this->RunSegmentationSaveIntermediateCheckButton->
      GetWidget()->SetCommand(this, "SaveIntermediateCallback");
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->RunSegmentationSaveIntermediateCheckButton->GetWidgetName());


  this->RunSegmentationSaveIntermediateCheckButton->
    GetWidget()->SetSelectedState(
      mrmlManager->GetSaveIntermediateResults());
  this->RunSegmentationSaveIntermediateCheckButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);


  // Create the working directory button

  if (!this->RunSegmentationDirectorySubFrame)
    {
    this->RunSegmentationDirectorySubFrame = vtkKWFrame::New();
    }
  if (!this->RunSegmentationDirectorySubFrame->IsCreated())
    {
      this->RunSegmentationDirectorySubFrame->SetParent( this->RunSegmentationSaveFrame->GetFrame());
      this->RunSegmentationDirectorySubFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->RunSegmentationDirectorySubFrame->GetWidgetName());


  if (!this->RunSegmentationDirectoryLabel)
    {
    this->RunSegmentationDirectoryLabel = vtkKWLabel::New();
    }

  if (!this->RunSegmentationDirectoryLabel->IsCreated())
    {
    this->RunSegmentationDirectoryLabel->SetParent(this->RunSegmentationDirectorySubFrame);
    this->RunSegmentationDirectoryLabel->Create();
    this->RunSegmentationDirectoryLabel->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RunSegmentationDirectoryLabel->SetText("Select Intermediate Directory: ");
    }

  this->Script( "pack %s -side left -anchor nw -padx 2 -pady 2", this->RunSegmentationDirectoryLabel->GetWidgetName());


  if (!this->RunSegmentationDirectoryButton)
    {
    this->RunSegmentationDirectoryButton = vtkKWLoadSaveButton::New();
    }
  if (!this->RunSegmentationDirectoryButton->IsCreated())
    {
      this->RunSegmentationDirectoryButton->SetParent(this->RunSegmentationDirectorySubFrame);
    this->RunSegmentationDirectoryButton->Create();
    this->RunSegmentationDirectoryButton->SetImageToPredefinedIcon(
      vtkKWIcon::IconFolderOpen);
    this->RunSegmentationDirectoryButton->TrimPathFromFileNameOn();
    this->RunSegmentationDirectoryButton->SetBalloonHelpString(
      "Select the directory where the intermediate results should be saved in");
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
    this->RunSegmentationDirectoryButton->SetText("Select");
    this->RunSegmentationDirectoryButton->GetLoadSaveDialog()->
      RetrieveLastPathFromRegistry("OpenPath");
    }

  this->RunSegmentationDirectoryButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script("pack %s -side left -anchor nw -fill x -padx 2 -pady 2", this->RunSegmentationDirectoryButton->GetWidgetName());

  // Postprocessing 
  if (!this->PostprocessingFrame)
    {
    this->PostprocessingFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->PostprocessingFrame->IsCreated())
    {
    this->PostprocessingFrame->SetParent(parent);
    this->PostprocessingFrame->Create();
    this->PostprocessingFrame->SetLabelText("Postprocessing");
    }

   if (this->GetGUI()->IsSegmentationModeAdvanced())
   { 
      this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->PostprocessingFrame->GetWidgetName());
   }

  if (!this->PostprocessingSubparcellationCheckButton)
    {
    this->PostprocessingSubparcellationCheckButton = vtkKWCheckButtonWithLabel::New();
    }
  if (!this->PostprocessingSubparcellationCheckButton->IsCreated())
    {
    this->PostprocessingSubparcellationCheckButton->SetParent(this->PostprocessingFrame->GetFrame());
    this->PostprocessingSubparcellationCheckButton->Create();
    this->PostprocessingSubparcellationCheckButton->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->PostprocessingSubparcellationCheckButton->SetLabelText("Subparcellation enabled:");
    this->PostprocessingSubparcellationCheckButton->GetWidget()->SetCommand(this, "PostprocessingSubparcellationCallback");
    }
  this->PostprocessingSubparcellationCheckButton->SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
  this->PostprocessingSubparcellationCheckButton->GetWidget()->SetSelectedState(mrmlManager->GetEnableSubParcellation());
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->PostprocessingSubparcellationCheckButton->GetWidgetName());

  if (!this->PostprocessingMinimumIslandSize)
    {
    this->PostprocessingMinimumIslandSize = vtkKWScaleWithEntry::New();
    }
  if (!this->PostprocessingMinimumIslandSize->IsCreated())
    {
    this->PostprocessingMinimumIslandSize->SetParent(this->PostprocessingFrame->GetFrame());
    this->PostprocessingMinimumIslandSize->PopupModeOn();
    this->PostprocessingMinimumIslandSize->Create();
    this->PostprocessingMinimumIslandSize->SetEntryWidth(4);
    this->PostprocessingMinimumIslandSize->SetLabelText("Minimum island size:");
    this->PostprocessingMinimumIslandSize->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 9);
    this->PostprocessingMinimumIslandSize->SetRange(1, 200);
    this->PostprocessingMinimumIslandSize->SetResolution(1);

    this->PostprocessingMinimumIslandSize->SetEndCommand(this, "PostprocessingMinimumIslandSizeCallback");
    this->PostprocessingMinimumIslandSize->SetEntryCommand(this, "PostprocessingMinimumIslandSizeCallback");
    this->PostprocessingMinimumIslandSize->GetEntry()->SetCommandTriggerToAnyChange();
    this->PostprocessingMinimumIslandSize->SetBalloonHelpString("Minimum island size in resulting label map");  
    }
  this->PostprocessingMinimumIslandSize->SetValue(mrmlManager->GetMinimumIslandSize());
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->PostprocessingMinimumIslandSize->GetWidgetName());

  if (!this->PostprocessingIsland2DCheckButton)
    {
    this->PostprocessingIsland2DCheckButton = vtkKWCheckButtonWithLabel::New();
    }
  if (!this->PostprocessingIsland2DCheckButton->IsCreated())
    {
    this->PostprocessingIsland2DCheckButton->SetParent(this->PostprocessingFrame->GetFrame());
    this->PostprocessingIsland2DCheckButton->Create();
    this->PostprocessingIsland2DCheckButton->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->PostprocessingIsland2DCheckButton->SetLabelText("2D Island Neighborhood :");
    this->PostprocessingIsland2DCheckButton->GetWidget()->SetCommand(this, "PostprocessingIsland2DFlagCallback");
    }
  this->PostprocessingIsland2DCheckButton->SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
  this->PostprocessingIsland2DCheckButton->GetWidget()->SetSelectedState(mrmlManager->GetIsland2DFlag());
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", this->PostprocessingIsland2DCheckButton->GetWidgetName());

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

 if (this->GetGUI()->IsSegmentationModeAdvanced())
   { 
      this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->RunSegmentationMiscFrame->GetWidgetName());
   }

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
    wizard_widget->GetOKButton()->SetText("Segment");
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
  // this->RemoveRunRegistrationOutputGUIObservers();
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

////---------------------------------------------------------------------------
//void vtkEMSegmentRunSegmentationStep::AddRunRegistrationOutputGUIObservers() 
//{
  //  this->RunSegmentationOutVolumeSelector->AddObserver(
  //  vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
  //  this->GetGUI()->GetGUICallbackCommand());  
//}

// //---------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::RemoveRunRegistrationOutputGUIObservers()
// {
//   this->RunSegmentationOutVolumeSelector->RemoveObservers(
//     vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
//     this->GetGUI()->GetGUICallbackCommand());  
// }
// 
// //---------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ProcessRunRegistrationOutputGUIEvents(
//   vtkObject *caller,
//   unsigned long event,
//   void *vtkNotUsed(callData)) 
// {
//   if (caller == this->RunSegmentationOutVolumeSelector && 
//       event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
//       this->RunSegmentationOutVolumeSelector->GetSelected() != NULL) 
//     { 
//     vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
//     if (mrmlManager)
//       {
//       mrmlManager->SetOutputVolumeMRMLID(
//         this->RunSegmentationOutVolumeSelector->GetSelected()->GetID());
//       }
//     }
// }

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SelectTemplateFileCallback()
{
  // The template file has changed because of user interaction

  if (!this->RunSegmentationSaveTemplateButton ||  
      !this->RunSegmentationSaveTemplateButton->IsCreated() || 
      (this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->GetStatus() != vtkKWDialog::StatusOK))
      {
        return;
      }

  //  vtkKWMessageDialog::PopupMessage(this->GetApplication(),NULL,"Info About Template Generation", "The template generator only works correctly if no more than one scene were imported! Visit web site listed under Help for more info." , vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer);

   this->RunSegmentationSaveTemplateButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
   vtksys_stl::string filename = this->RunSegmentationSaveTemplateButton->GetFileName();
   vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
   if (mrmlManager)
     {
        mrmlManager->SetSaveTemplateFilename(filename.c_str());
        if (mrmlManager->CreateTemplateFile())
        {
              vtkKWMessageDialog::PopupMessage(this->GetApplication(),
                                     NULL,
                                     "Create Template Error",
                                     "Could not create template - the filename of the template was probably not in the same directory as the scene",
                                     vtkKWMessageDialog::ErrorIcon | 
                                     vtkKWMessageDialog::InvokeAtPointer); 
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
void vtkEMSegmentRunSegmentationStep::PostprocessingSubparcellationCallback(int state)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
      mrmlManager->SetEnableSubParcellation(state);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::PostprocessingMinimumIslandSizeCallback(float value)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
      mrmlManager->SetMinimumIslandSize(value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::PostprocessingIsland2DFlagCallback(int state)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
      mrmlManager->SetIsland2DFlag(state);
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

  vtkEMSegmentLogic *Logic = this->GetGUI()->GetLogic();
  vtkEMSegmentMRMLManager* mrmlManager = this->GetGUI()->GetMRMLManager();

  if (!mrmlManager)
    {
    return;
    }
  
  // make sure that data types are the same,
  // it is ok if the 'input target node' and the 'atlas node' are using different types,
  // but the 'aligned atlas node'(after register/resample/cast) has to be of the same type as the 'input target node'
  vtkMRMLEMSVolumeCollectionNode* targetNode =
    mrmlManager->GetWorkingDataNode()->GetInputTargetNode();
  vtkMRMLEMSAtlasNode* alignedAtlasNode =
    mrmlManager->GetWorkingDataNode()->GetAlignedAtlasNode();

  if (!mrmlManager->DoTargetAndAtlasDataTypesMatch(targetNode, alignedAtlasNode))
    {
    // popup an error message
    std::string errorMessage = 
      "Scalar type mismatch for input images; all image scalar types must be "
      "the same (including input channels and aligned/resampled/casted atlas images).";
    vtkErrorMacro(<< errorMessage);
    vtkKWMessageDialog::PopupMessage(this->GetApplication(),
                                     NULL,
                                     "Input Image Error",
                                     errorMessage.c_str(),
                                     vtkKWMessageDialog::ErrorIcon | 
                                     vtkKWMessageDialog::InvokeAtPointer);    
    return;
    }


  if (mrmlManager->GetSaveIntermediateResults() && (!mrmlManager->GetSaveWorkingDirectory() || !vtksys::SystemTools::FileExists(mrmlManager->GetSaveWorkingDirectory())))  
   {
     std::string errorMessage = 
       "Saving Intermediate Results is turned on but " ;
     if (!mrmlManager->GetSaveWorkingDirectory() || !strcmp(mrmlManager->GetSaveWorkingDirectory(),"NULL")) 
      {
      errorMessage += "no intermediate directory is selected!";
      }
     else
       {
     errorMessage += "the intermediate directory " + std::string(mrmlManager->GetSaveWorkingDirectory()) + " does not exist!";
       }

     vtkErrorMacro(<< errorMessage);
     vtkKWMessageDialog::PopupMessage(this->GetApplication(),
                                     NULL,
                                     "Save Intermediate Result Error",
                                     errorMessage.c_str(),
                                     vtkKWMessageDialog::ErrorIcon | 
                                     vtkKWMessageDialog::InvokeAtPointer);    
     
    return;

    }
  // Compute Number of Training Samples if they are not set
  if (this->GetGUI()->GetMRMLManager()->GetAtlasNumberOfTrainingSamples() <= 0 )
    {
      this->GetGUI()->GetMRMLManager()->ComputeAtlasNumberOfTrainingSamples();
    }

  // Create output volume 
  this->GetGUI()->GetLogic()->CreateOutputVolumeNode();

  // start the segmentation
  vtkKWProgressDialog* progress = vtkKWProgressDialog::New();
  progress->SetParent(this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow());
  progress->SetMasterWindow(this->GetGUI()->GetApplicationGUI()->GetMainSlicerWindow());
  progress->Create();
  progress->SetMessageText("Please wait until segmentation has finished.");
  progress->Display();
  int returnCode = Logic->StartSegmentationWithoutPreprocessing(this->GetGUI()->GetApplicationLogic());
  progress->SetParent(NULL);
  progress->Delete();

  std::string error_msg = Logic->GetErrorMessage();
  if (returnCode != EXIT_SUCCESS || error_msg.size() > 0)
    {
    vtkKWMessageDialog::PopupMessage(this->GetApplication(), NULL,
      "Segmentation Error", error_msg.c_str(),
      vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer
    );
    vtkErrorMacro(<< "Segmentation did not execute correctly"); 
    return;
    }

  // display Results 
  vtkSlicerApplicationGUI *applicationGUI  = this->GetGUI()->GetApplicationGUI();
  vtkMRMLScalarVolumeNode* output = this->GetGUI()->GetMRMLManager()->GetOutputVolumeNode();
 
  vtkSlicerVolumesGUI *vgui = vtkSlicerVolumesGUI::SafeDownCast (vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName ( "Volumes"));
   if (!vgui)  
    {
      vtkErrorMacro("CreateOutputVolumeNode: could not find vtkSlicerVolumesGUI "); 
      return;
    }
    // If you do not do this first you get all kind of wired errors whne setting the foreground !
   vgui->GetVolumeSelectorWidget()->SetSelected(output);

  applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetForegroundSelector()->SetSelected(output);
  applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetForegroundSelector()->SetSelected(output);
  applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetForegroundSelector()->SetSelected(output);
  
  // only the background is shown
  if (!applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->GetValue())
      {
      applicationGUI->GetSlicesControlGUI()->GetSliceFadeScale()->SetValue(0.5);
     }
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

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::InitialROIWidget() 
{
  this->DefineVOIFrame  = NULL;
  this->FrameButtons    = NULL;
  this->ButtonsShow     = NULL;
  this->ButtonsReset    = NULL;
  this->ROILabelMapNode = NULL;
  this->ROILabelMap     = NULL;
  // this->ROIHideFlag     = 0; 

  this->roiNode = NULL;
  this->roiWidget = NULL;
  this->roiUpdateGuard = false;

  this->ROIMRMLCallbackCommand = vtkCallbackCommand::New();
  this->ROIMRMLCallbackCommand->SetClientData(reinterpret_cast<void*>(this));
  this->ROIMRMLCallbackCommand->SetCallback(vtkEMSegmentRunSegmentationStep::ROIMRMLCallback);

  this->VisibilityIcons = NULL;
  this->VisibilityLabel = NULL;

  this->WizardGUICallbackCommand = vtkCallbackCommand::New();
  this->WizardGUICallbackCommand->SetClientData(reinterpret_cast<void *>(this));
  this->WizardGUICallbackCommand->SetCallback(vtkEMSegmentRunSegmentationStep::WizardGUICallback);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ResetROIWidget() 
{
 if(this->WizardGUICallbackCommand) 
  {
        this->WizardGUICallbackCommand->Delete();
        this->WizardGUICallbackCommand=NULL;
  }

  if (this->DefineVOIFrame)
  {
    this->DefineVOIFrame->Delete();
    this->DefineVOIFrame = NULL;
  }

  if (this->FrameButtons)
  {
    this->FrameButtons->Delete();
    this->FrameButtons = NULL;
  }

  if (this->ButtonsShow) {
    this->ButtonsShow->Delete();
    this->ButtonsShow= NULL;
  }

  if (this->ButtonsReset) {
    this->ButtonsReset->Delete();
    this->ButtonsReset= NULL;
  }

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove();

  if(this->roiNode)
  {
    this->roiNode->Delete();
    this->roiNode = NULL;
  }

  if(this->roiWidget)
    {
    this->roiWidget->Delete();
    this->roiWidget = NULL;
    }

  if (this->ROIMRMLCallbackCommand)
    {
    this->ROIMRMLCallbackCommand->Delete();
    this->ROIMRMLCallbackCommand = NULL;
    }
  if ( this->VisibilityIcons ) {
    this->VisibilityIcons->Delete  ( );
    this->VisibilityIcons = NULL;
    }
  if ( this->VisibilityLabel ) {
    this->VisibilityLabel->SetParent(NULL);
    this->VisibilityLabel->Delete();
    this->VisibilityLabel = NULL;
  }

}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ShowROIGUI(vtkKWWidget* parent)
{
  // cout << "vtkEMSegmentRunSegmentationStep::ShowUserInterface() Start " << endl;
  // ----------------------------------------
  // Display Scan1, Delete Super Sampled and Grid  
  // ----------------------------------------
  //  vtkMRMLEMSegmentRunSegmentationStepNode* node = this->GetGUI()->GetNode();

  // Make sure that something is displayed otherwise 3D Viewer is not set correctly when defining ROI Widget 
  // vtkSlicerViewControlGUI cGUI = this->GetGUI()->GetApplicationGUI()->GetViewControlGUI;
  // cGUI->GetRedSliceNode->
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  int dimensions[3]; 
  vtkMRMLVolumeNode* volumeNode = mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
   memcpy(dimensions,volumeNode->GetImageData()->GetDimensions(),sizeof(int)*3);
  // Changes all viewers to set to input volime
  // this->SetRedGreenYellowAllVolumes(this->GetGUI()->GetApplicationGUI(), volumeNode->GetID());

  // Define GUI 
  if (!this->DefineVOIFrame)
    {
    this->DefineVOIFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->DefineVOIFrame->IsCreated())
    {
    this->DefineVOIFrame->SetParent(parent);
    this->DefineVOIFrame->Create();
    this->DefineVOIFrame->SetLabelText("Define VOI");
    }

 if (this->GetGUI()->IsSegmentationModeAdvanced())
   { 
     this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2", this->DefineVOIFrame->GetWidgetName());
   }

  if(!this->roiNode)
    {
      // Create ROI MRML node
      // see Base/GUI/vtkSlicerNodeSelectorWidget.cxx:ProcessNewNodeCommand
      // First find out if roi node in scene 
      vtkMRMLScene *scene = this->GetGUI()->GetLogic()->GetMRMLScene();
      int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLROINode");
      const char nodeName[40] = "SegmentationROI";
      // cout << "Blkub " << numNodes << endl;
      for (int i = 0; i < numNodes; ++i)
    {
      vtkMRMLROINode* node = (vtkMRMLROINode*)  scene->GetNthNodeByClass(i, "vtkMRMLROINode");
      // cout << "Blkub-- " << i  << " " << (node != NULL)  !node->GetName() && !strcmp(node->GetName(), nodeName) << endl;

      if (node != NULL && node->GetName() && !strcmp(node->GetName(), nodeName) )
        {
          // This does not work - so  have to delete it 
          //this->roiNode = node;
          scene->RemoveNode(node);
        // break;
        }
    }
      // Create new roi if necessary
      this->roiNode = static_cast<vtkMRMLROINode*>(scene->CreateNodeByClass("vtkMRMLROINode"));
      this->roiNode->SetName("SegmentationROI");
      scene->AddNode(this->roiNode);
      this->roiNode->SetVisibility(0);
    }
  
  // update the roiNode ROI to reflect what is stored in ROI MRML
  // Originally was the wrong way aroung roiNode is not defined !
  // this->MRMLUpdateROIFromROINode();
  // Here SegmentationBoundary* sets roiNode
  this->MRMLUpdateROINodeFromROI();

  // this->roiNode->SetVisibility(this->ROIHideFlag);
  this->roiNode->AddObserver(vtkCommand::ModifiedEvent, this->ROIMRMLCallbackCommand);

  // put 
  // InitROIRender();
  // Very Important 

  if (!this->roiWidget)
    {
    this->roiWidget = vtkSlicerROIDisplayWidget::New();
    }

  if (!this->roiWidget->IsCreated())
    {
    double rasDimensions[3];
    double rasCenter[3];
    double rasBounds[6];

    vtkSlicerSliceLogic::GetVolumeRASBox(volumeNode, rasDimensions, rasCenter);
    //cout << "Center : " << rasCenter[0] << " " << rasCenter[1] << " " << rasCenter[2] << endl;
    //cout << "Dimension: " << rasDimensions[0] << " " << rasDimensions[1] << " " << rasDimensions[2] << endl;

    rasBounds[0] = min(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
    rasBounds[1] = min(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
    rasBounds[2] = min(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    rasBounds[3] = max(rasCenter[0]-rasDimensions[0]/2.,rasCenter[0]+rasDimensions[0]/2.);
    rasBounds[4] = max(rasCenter[1]-rasDimensions[1]/2.,rasCenter[1]+rasDimensions[1]/2.);
    rasBounds[5] = max(rasCenter[2]-rasDimensions[2]/2.,rasCenter[2]+rasDimensions[2]/2.);
    
    this->roiWidget->SetParent(this->DefineVOIFrame->GetFrame());
    this->roiWidget->Create();

    this->roiWidget->SetXRangeExtent(rasBounds[0],rasBounds[3]);
    this->roiWidget->SetYRangeExtent(rasBounds[1],rasBounds[4]);
    this->roiWidget->SetZRangeExtent(rasBounds[2],rasBounds[5]);

    this->roiWidget->SetXResolution(fabs(rasBounds[3]-rasBounds[0])/100.);
    this->roiWidget->SetYResolution(fabs(rasBounds[4]-rasBounds[1])/100.);
    this->roiWidget->SetZResolution(fabs(rasBounds[5]-rasBounds[2])/100.);

    this->roiWidget->SetROINode(this->roiNode);
    }
  this->Script("pack %s -side top -anchor nw -padx 2 -pady 3 -fill x",
               this->roiWidget->GetWidgetName());

  if (!this->FrameButtons)
    {
    this->FrameButtons = vtkKWFrame::New();
    }
  if (!this->FrameButtons->IsCreated())
    {
      this->FrameButtons->SetParent(this->DefineVOIFrame->GetFrame());
    this->FrameButtons->Create();
    // this->FrameButtons->SetLabelText("");
    // define buttons 
  }
  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 0", this->FrameButtons->GetWidgetName());
 
  if (!this->ButtonsShow) {
    this->ButtonsShow = vtkKWPushButton::New();
  }

  if(!this->VisibilityIcons) 
    {
      this->VisibilityIcons = vtkSlicerVisibilityIcons::New ( );
    }

  if (!this->ButtonsShow->IsCreated()) {
    this->ButtonsShow->SetParent(this->FrameButtons);
    this->ButtonsShow->Create();
    this->ButtonsShow->SetText("Show render");
    this->ButtonsShow->SetReliefToFlat ( );
    this->ButtonsShow->SetOverReliefToNone ( );
    this->ButtonsShow->SetBorderWidth ( 0 );
    this->ButtonsShow->SetBalloonHelpString("Show/hide ROI overlay in image viewer"); 
    if (this->ROILabelMapNode) 
      {
      this->ROIMapShow();
      } 
    else 
     { 
      this->ROIMapRemove();
    }
  }

  if(!this->VisibilityLabel){
    this->VisibilityLabel = vtkKWLabel::New();
  }
  if(!this->VisibilityLabel->IsCreated()){
    this->VisibilityLabel->SetParent (this->FrameButtons);
    this->VisibilityLabel->Create();
    this->VisibilityLabel->SetText("Display VOI in 2D Viewer");
  }
 if (!this->ButtonsReset) {
    this->ButtonsReset = vtkKWPushButton::New();
  }
  if (!this->ButtonsReset->IsCreated()) {
    this->ButtonsReset->SetParent(this->FrameButtons);
    this->ButtonsReset->Create();
    this->ButtonsReset->SetWidth(20);
    this->ButtonsReset->SetText("Reset");
    this->ButtonsReset->SetBalloonHelpString("Reset Values"); 
  }
  //   this->Script("pack %s %s %s -side left -anchor w -padx 4 -pady 2", this->VisibilityLabel->GetWidgetName(), this->ButtonsShow->GetWidgetName(), this->ButtonsReset->GetWidgetName());
  this->Script("pack %s %s -side left -anchor w -padx 4 -pady 2", this->VisibilityLabel->GetWidgetName(), this->ButtonsShow->GetWidgetName());

  {
   vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget(); 
   wizard_widget->BackButtonVisibilityOn();
   wizard_widget->GetCancelButton()->EnabledOn();
  }

  //vtkIndent ind;
  //this->roiNode->PrintSelf(cout,ind);

  this->AddGUIObservers();
  // Keep separate bc GUIObserver is also called from vtkEMSegmentRunSegmentationStepGUI ! 
  // You only want to add the observers below when the step is active 
  // this->AddROISamplingGUIObservers();
  // cout << "vtkEMSegmentRunSegmentationStep::ShowUserInterface() end " << endl;
}

// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ROIXChangedCallback(double min, double max)  
// {
//   this->ROIChangedCallback(0,min, max);
// }
// 
// 
// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ROIYChangedCallback(double min, double max)  
// {
//   this->ROIChangedCallback(1,min, max);
// }
// 
// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ROIZChangedCallback(double min, double max)  
// {
//   this->ROIChangedCallback(2,min, max);
// }  
// 
// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ROIChangedCallback(int axis, double min, double max)  
// {
//   vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
//   if (!mrmlManager) return;
//  
//   {
//     int minPoint[3];
//     mrmlManager->GetSegmentationBoundaryMin(minPoint);
//     minPoint[axis] = min;
//     mrmlManager->SetSegmentationBoundaryMin(minPoint);
//   }
// 
//   {
//     int maxPoint[3];
//     mrmlManager->GetSegmentationBoundaryMax(maxPoint);
//     maxPoint[axis] = max;
//     mrmlManager->SetSegmentationBoundaryMax(maxPoint);
//   }
//   this->ROIMapUpdate();
// }



//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::AddGUIObservers() 
{
  // cout << "vtkEMSegmentRunSegmentationStep::AddGUIObservers()" << endl; 
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->ButtonsShow && (!this->ButtonsShow->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsShow->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 

  if (this->ButtonsReset && (!this->ButtonsReset->HasObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand))) 
    {
      this->ButtonsReset->AddObserver(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand );  
    } 
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::AddROISamplingGUIObservers() {
  vtkRenderWindowInteractor *rwi0 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi0->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand, 1);
  rwi0->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand, 1);

  // Slice GUI 1

  vtkRenderWindowInteractor *rwi1 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi1->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent,this->WizardGUICallbackCommand, 1);
  rwi1->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent,this->WizardGUICallbackCommand, 1);

  // Slice GUI 2

  vtkRenderWindowInteractor *rwi2 = vtkSlicerApplicationGUI::SafeDownCast(
    this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green")->
    GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();

  rwi2->GetInteractorStyle()->AddObserver(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand, 1);
  rwi2->GetInteractorStyle()->AddObserver(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand, 1);
} 

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RemoveGUIObservers() 
{
  if (this->ButtonsShow) 
    {
      this->ButtonsShow->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
    }

  if (this->ButtonsReset) 
  {
      this->ButtonsReset->RemoveObservers(vtkKWPushButton::InvokedEvent, this->WizardGUICallbackCommand);  
  }
  this->RemoveROISamplingGUIObservers();
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RemoveROISamplingGUIObservers() {
  if (!this->GetGUI()) return;
  vtkSlicerApplicationGUI *ApplicationGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI());
  if (!ApplicationGUI) return; 
  for (int i = 0 ; i < 3 ; i ++ ) {
    vtkSlicerSliceGUI *MainGUI = NULL;
    if (i == 0) MainGUI = ApplicationGUI->GetMainSliceGUI("Red");
    if (i == 1) MainGUI = ApplicationGUI->GetMainSliceGUI("Yellow");
    if (i == 2) MainGUI = ApplicationGUI->GetMainSliceGUI("Green");
    if (!MainGUI) return;
    vtkRenderWindowInteractor *rwi = MainGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    rwi->GetInteractorStyle()->RemoveObservers(vtkCommand::LeftButtonPressEvent, this->WizardGUICallbackCommand);
    rwi->GetInteractorStyle()->RemoveObservers(vtkCommand::RightButtonPressEvent, this->WizardGUICallbackCommand);
  }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData )
{
    vtkEMSegmentRunSegmentationStep *self = reinterpret_cast<vtkEMSegmentRunSegmentationStep *>(clientData);
    if (self) { self->ProcessGUIEvents(caller, event, callData); }


}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIReset() {
  cout << "ROIReset Start" << endl;
  //this->ROIHideFlag = 0;
  // this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());

  if (!this->roiNode) 
    {
      return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
     {
     return;
     }
   vtkMRMLVolumeNode* volumeNode = mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
   if (!volumeNode)
     {
       return;
     }

   int minIJK[3] = {0, 0, 0};
   mrmlManager->GetSegmentationBoundaryMin(minIJK);
   minIJK[0] -= 1;   minIJK[1] -= 1;   minIJK[2] -= 1;

   int maxIJK[3] = {0, 0, 0};
   mrmlManager->GetSegmentationBoundaryMin(maxIJK);
   maxIJK[0] -= 1;   maxIJK[1] -= 1;   maxIJK[2] -= 1;

   cout << "MIN IJK : " <<   minIJK[0] << " " <<   minIJK[1] << " " <<   minIJK[2] << endl;
   cout << "Max IJK : " <<   maxIJK[0] << " " <<   maxIJK[1] << " " <<   maxIJK[2] << endl;
   double minRAS[3];
   double maxRAS[3];
   vtkEMSegmentLogic::TransferIJKToRAS(volumeNode,minIJK,minRAS);
   vtkEMSegmentLogic::TransferIJKToRAS(volumeNode,maxIJK,maxRAS);
   cout << "MIN RAS : " <<   minRAS[0] << " " <<   minRAS[1] << " " <<   minRAS[2] << endl;
   cout << "Max RAS : " <<   maxRAS[0] << " " <<   maxRAS[1] << " " <<   maxRAS[2] << endl;

   double XYZ[3] = {(maxRAS[0]  + minRAS[0])/2,  (maxRAS[1]  + minRAS[1])/2, (maxRAS[2]  + minRAS[2])/2};
   double radius[3] = {maxRAS[0]  - XYZ[0], maxRAS[1]  - XYZ[1], maxRAS[2]  - XYZ[2]};
   cout << " REset " << XYZ[0] << " " << XYZ[1] << " " << XYZ[2] << endl;
   this->roiNode->SetXYZ(XYZ[0],XYZ[1],XYZ[2]);
   this->roiNode->SetRadiusXYZ(radius[0],radius[1],radius[2]);
  cout << "ROIReset Reset" << endl;
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIUpdateAxisWithNewSample(vtkKWRange *ROIAxis, int Sample) {
  if (!ROIAxis) return;
  double *oldRange = ROIAxis->GetRange();
  double newRange[2];

  if ((Sample < oldRange[0]) || (oldRange[0] < 0)) newRange[0] = Sample;
  else  newRange[0] = oldRange[0]; 
  if ((Sample > oldRange[1]) || (oldRange[1] < 0)) newRange[1] = Sample;
  else newRange[1] = oldRange[1]; 
  ROIAxis->SetRange(newRange);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIUpdateWithNewSample(double rasSample[3]) {
  double *roiXYZ = roiNode->GetXYZ();
  double *roiRadius = roiNode->GetRadiusXYZ();
  double dXYZ[3], shiftXYZ[3];
  int i;

  for(i=0;i<3;i++)
    {
    dXYZ[i] = fabs(roiXYZ[i]-rasSample[i]);
    shiftXYZ[i] = fabs(dXYZ[i]-roiRadius[i])/2.;
    }

  if(dXYZ[0]>roiRadius[0] || dXYZ[1]>roiRadius[1] || dXYZ[2]>roiRadius[2])
    {
    // click outside the box
    for(i=0;i<3;i++)
      {
      if(dXYZ[i]>roiRadius[i])
        {
        if(rasSample[i]>roiXYZ[i])
          {
          roiXYZ[i] = roiXYZ[i]+shiftXYZ[i];
          }
        else
          {
          roiXYZ[i] = roiXYZ[i]-shiftXYZ[i];
          }
        roiRadius[i] = roiRadius[i]+shiftXYZ[i];
        }
      }
    }
  // in-slice reduction of ROI size is not there yet, because need to pass
  // clickXY to this function

  this->roiNode->SetXYZ(roiXYZ);
  this->roiNode->SetRadiusXYZ(roiRadius);
  // do not update segmentationaboundary node until we transition - that way we can do a true reset
}

// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ROIUpdateAxisWithNode(vtkEMSegmentMRMLManager *mrmlManager, vtkKWRange *ROIAxis, int Axis) {
//   if (!mrmlManager || !ROIAxis) return;
//   int minPoint[3];
//   mrmlManager->GetSegmentationBoundaryMin(minPoint);
//   int maxPoint[3];
//   mrmlManager->GetSegmentationBoundaryMin(maxPoint);
//   ROIAxis->SetRange(minPoint[Axis],maxPoint[Axis]);
// }

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIUpdateWithNode() {
  // cout << "ROIUpdateWithNode Start" << endl;
  this->ROIMapUpdate();
}


//----------------------------------------------------------------------------
// Return 1 if it is a valid ROI and zero otherwise
int vtkEMSegmentRunSegmentationStep::ROICheck() {
  // Define Variables
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager) return 0;
  int minPoint[3];
  int maxPoint[3];
  mrmlManager->GetSegmentationBoundaryMax(maxPoint);
  mrmlManager->GetSegmentationBoundaryMin(minPoint);
  // Don't change here bc min and max points are only compared in boundary mode - not IJK mode 
  return this->ROICheck(minPoint,maxPoint);
} 


int vtkEMSegmentRunSegmentationStep::ROICheck(int minPoint[3], int maxPoint[3]) 
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager) return 0;

  vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0); 
  if (!volumeNode) return 0;

  if (!volumeNode || !volumeNode->GetImageData()) {
    cout << "vtkChangeTrackerLogic::CheckROI: No Volume Defined" << endl;
    return 0;
  }

  int* dimensions = volumeNode->GetImageData()->GetDimensions();
  for (int i = 0 ; i < 3 ; i++) {
    // boundary starts at 1 not like IJK at 0 ! 
    if ((minPoint[i] < 1) || (minPoint[i] > dimensions[i])) {
      // cout << "vtkChangeTrackerLogic::CheckROI: dimension["<<i<<"] = " << dimensions[i] << endl;
      return 0 ;
    }
    if ((maxPoint[i] < 1) || (maxPoint[i] > dimensions[i])) return 0 ;
    if (maxPoint[i] < minPoint[i]) return 0;
  }
  return 1;
}



//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIMapUpdate() {

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  int minPoint[3];
  mrmlManager->GetSegmentationBoundaryMin(minPoint);
  minPoint[0] -= 1;  minPoint[1] -= 1;  minPoint[2] -= 1;
  int maxPoint[3];
  mrmlManager->GetSegmentationBoundaryMax(maxPoint);
  maxPoint[0] -= 1;  maxPoint[1] -= 1;  maxPoint[2]  -= 1;

  if (!this->ROILabelMapNode || !this->ROILabelMap || !this->ROICheck()) return;
  int size[3]   = {maxPoint[0] - minPoint[0] + 1, 
                   maxPoint[1] - minPoint[1] + 1, 
                   maxPoint[2] - minPoint[2] + 1};

  int center[3] = {(maxPoint[0] + minPoint[0])/2,
                   (maxPoint[1] + minPoint[1])/2, 
                   (maxPoint[2] + minPoint[2])/2};

  this->ROILabelMap->SetCenter(center);
  this->ROILabelMap->SetSize(size);
  this->ROILabelMap->Update();
  this->ROILabelMapNode->Modified();

  // Update the roiNode
  if(roiNode && !roiUpdateGuard)
    {
    roiUpdateGuard = true;
    MRMLUpdateROINodeFromROI();
    roiNode->Modified();
    roiUpdateGuard = false;
    }
  
  double *roiXYZ = roiNode->GetXYZ();      
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(roiXYZ[0], roiXYZ[1], roiXYZ[2]);

}

//----------------------------------------------------------------------------
int vtkEMSegmentRunSegmentationStep::ROIMapShow() {
  // -----
  // Initialize
  if (!this->ROICheck()) {
    vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
                                     this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
                                     "Change Tracker", 
                                     "Please define VOI correctly before pressing button", 
                                     vtkKWMessageDialog::ErrorIcon);
    return 0;
  }
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager) return 0;

  vtkMRMLScene* mrmlScene           =  mrmlManager->GetMRMLScene();
  vtkMRMLVolumeNode* volumeNode     =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0); 
  if (!volumeNode) return 0;
  int* dimensions = volumeNode->GetImageData()->GetDimensions();

  if (this->ROILabelMapNode || this->ROILabelMap) this->ROIMapRemove(); 

  // -----
  // Define LabelMap 
  this->ROILabelMap =  vtkImageRectangularSource::New();
  this->ROILabelMap->SetWholeExtent(0,dimensions[0] -1,0,dimensions[1] -1, 0,dimensions[2] -1); 
  this->ROILabelMap->SetOutputScalarTypeToShort();
  this->ROILabelMap->SetInsideGraySlopeFlag(0); 
  this->ROILabelMap->SetInValue(17);
  this->ROILabelMap->SetOutValue(0);
  this->ROILabelMap->Update();

  // Show map in Slicer 3 
  //  set scene [[$this GetLogic] GetMRMLScene]
  //  set volumesLogic [$::slicer3::VolumesGUI GetLogic]
  vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkSlicerApplicationGUI *applicationGUI = this->GetGUI()->GetApplicationGUI();
//  vtkSlicerApplicationLogic *applicationLogic = this->GetGUI()->GetLogic()->GetApplicationLogic();

  vtkSlicerVolumesGUI  *volumesGUI    = vtkSlicerVolumesGUI::SafeDownCast(application->GetModuleGUIByName("Volumes")); 
  vtkSlicerVolumesLogic *volumesLogic = volumesGUI->GetLogic();
  // set labelNode [$volumesLogic CreateLabelVolume $scene $volumeNode $name]
  this->ROILabelMapNode = volumesLogic->CreateLabelVolume(mrmlScene,volumeNode, "EM_ROI");
  this->ROILabelMapNode->SetAndObserveImageData(this->ROILabelMap->GetOutput());

  // Now show in foreground 
  //  make the source node the active background, and the label node the active label
  // set selectionNode [[[$this GetLogic] GetApplicationLogic]  GetSelectionNode]
  //$selectionNode SetReferenceActiveVolumeID [$volumeNode GetID]
  //$selectionNode SetReferenceActiveLabelVolumeID [$labelNode GetID] 
  //  applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(volumeNode->GetID());
 
  // Reset to original slice location 
  double oldSliceSetting[3];
  oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceOffset());
  oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceOffset());
  oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceOffset());

  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID(this->ROILabelMapNode->GetID());

  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetForegroundOpacity(0.6);

  // Reset to original slice location 
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->SetSliceOffset(oldSliceSetting[1]);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->SetSliceOffset(oldSliceSetting[2]);


  if (this->ButtonsShow && this->ButtonsShow->IsCreated())  {
    this->ButtonsShow->SetText("Hide render");
    if (this->VisibilityIcons) {
      this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetVisibleIcon());
    }
  }
  this->AddROISamplingGUIObservers();
  
  this->ROIMapUpdate();
  return 1;
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIMapRemove() {
  
  if (this->ROILabelMapNode && this->GetGUI() && this->GetGUI()->GetMRMLScene())  { 
    this->GetGUI()->GetMRMLScene()->RemoveNode(this->ROILabelMapNode);
  }
  this->ROILabelMapNode = NULL;

  if (this->ROILabelMap) { 
    this->ROILabelMap->Delete();
    this->ROILabelMap = NULL;
  }

  // Needs to be done otherwise when going backwards field is not correctly defined   
  if (this->ButtonsShow && this->ButtonsShow->IsCreated())  {
    this->ButtonsShow->SetText("Show render");
    if (this->VisibilityIcons) {
      this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
    }
  }
  this->RemoveROISamplingGUIObservers();
  
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::RetrieveInteractorCoordinates(vtkSlicerSliceGUI *sliceGUI, 
                                                               vtkRenderWindowInteractor *rwi,
                                                               int coordsIJK[3], double coordsRAS[3]) 
{
  coordsIJK[0] = coordsIJK[1] = coordsIJK[2] = -1;
  coordsRAS[0] = coordsRAS[1] = coordsRAS[2] = -1.;
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    { 
      cout << "ERROR: vtkEMSegmentRunSegmentationStep::RetrieveInteractorCoordinates: No MRML Manager" << endl;
      return;
    } 

  vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0); 
  if (!volumeNode)
    {
      cout << "ERROR: vtkEMSegmentRunSegmentationStep::RetrieveInteractorCoordinates: No Input target node" << endl;
      return;
    }

  // --------------------------------------------------------------
  // Compute RAS coordinates
  int point[2];
  rwi->GetLastEventPosition(point);
  double inPt[4] = {point[0], point[1], 0, 1};
  double rasPt[4];
  vtkMatrix4x4 *matrix = sliceGUI->GetLogic()->GetSliceNode()->GetXYToRAS();
  matrix->MultiplyPoint(inPt, rasPt); 
  matrix = NULL;

  coordsRAS[0] = rasPt[0];
  coordsRAS[1] = rasPt[1];
  coordsRAS[2] = rasPt[2];

  // --------------------------------------------------------------
  // Compute IJK coordinates
  double ijkPt[4];
  vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(rasPt, ijkPt);
  rasToijk->Delete();

  // --------------------------------------------------------------
  // Check validity of coordinates
  int* dimensions = volumeNode->GetImageData()->GetDimensions();
  for (int i = 0 ; i < 3 ; i++) {
    if (ijkPt[i] < 0 ) ijkPt[i] = 0;
    else if (ijkPt[i] >=  dimensions[i] ) ijkPt[i] = dimensions[i] -1;    
  }
  //coordsIJK[0] = int(0.5+(ijkPt[0]));  
  //coordsIJK[1] = int(0.5+(ijkPt[1])); 
  //coordsIJK[2] = int(0.5+(ijkPt[2])); 
  coordsIJK[0] = int(ijkPt[0]);  
  coordsIJK[1] = int(ijkPt[1]); 
  coordsIJK[2] = int(ijkPt[2]); 

  //cout << "Sample:  " << rasPt[0] << " " <<  rasPt[1] << " " << rasPt[2] << " " << rasPt[3] << endl;
  //cout << "Coord: " << coords[0] << " " << coords[1] << " " << coords[2] << " " << coords[3] << endl;
  //cout << "Dimen: " << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << " " <<  endl;

}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ProcessGUIEvents(vtkObject *caller, unsigned long event,
                                               void *vtkNotUsed(callData)) {
  
  if (event == vtkKWPushButton::InvokedEvent) {
    vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
    if (this->ButtonsShow && (button == this->ButtonsShow)) 
    { 
      if (this->ROILabelMapNode) {
        this->ROIMapRemove();
      } else { 
        if (this->ROIMapShow()) { 
          //this->UpdateROIRender();
        }
// FIXME: when feature complete
//        if (roiNode)  
//          roiNode->SetVisibility(1);
      }
    }
    if (this->ButtonsReset && (button == this->ButtonsReset)) 
    { 
      if (this->ROILabelMapNode) {
        this->ButtonsShow->SetText("Show render");
        this->ROIMapRemove();
        roiNode->SetVisibility(0);
        this->ButtonsShow->SetImageToIcon(this->VisibilityIcons->GetInvisibleIcon());
        // this->ResetROIRender();
      }
      this->ROIReset();
      this->MRMLUpdateROIFromROINode();
    }
    return;
  }

  vtkSlicerInteractorStyle *s = vtkSlicerInteractorStyle::SafeDownCast(caller);
  if (s &&  this->ROILabelMapNode)
  {
    // Retrieve Coordinates and update ROI
    int index = 0; 
    vtkSlicerSliceGUI *sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(
      this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Red");

    vtkRenderWindowInteractor *rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    while (index < 2 && (s != rwi->GetInteractorStyle())) {
        index ++;
        if (index == 1) {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Yellow");
        } else {
          sliceGUI = vtkSlicerApplicationGUI::SafeDownCast(this->GetGUI()->GetApplicationGUI())->GetMainSliceGUI("Green");
        }
        rwi = sliceGUI->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
    }
    int ijkCoords[3];
    double rasCoords[3];
    this->RetrieveInteractorCoordinates(sliceGUI, rwi, ijkCoords, rasCoords);
    
    if(event == vtkCommand::LeftButtonPressEvent)
      {
      this->ROIUpdateWithNewSample(rasCoords);
      }
    else if(event == vtkCommand::RightButtonPressEvent)
      {
      this->roiNode->SetXYZ(rasCoords);
      }
    MRMLUpdateROINodeFromROI();
  }
}

//----------------------------------------------------------------------------
 void vtkEMSegmentRunSegmentationStep::ProcessMRMLEvents(vtkObject *caller, unsigned long event,
                             void *vtkNotUsed(callData))
 {
 //  if(event == vtkCommand::ModifiedEvent){
     vtkMRMLROINode *roiCaller = vtkMRMLROINode::SafeDownCast(caller);
     if(roiCaller && roiCaller == this->roiNode && event == vtkCommand::ModifiedEvent && !roiUpdateGuard)
       {
       
       roiUpdateGuard = true;
       double *roiXYZ = roiNode->GetXYZ();
       double *roiRadius = roiNode->GetRadiusXYZ();
       double newROIXYZ[3], newROIRadius[3];
       newROIXYZ[0] = ceil(roiXYZ[0]);
       newROIXYZ[1] = ceil(roiXYZ[1]);
       newROIXYZ[2] = ceil(roiXYZ[2]);
       newROIRadius[0] = ceil(roiRadius[0]);
       newROIRadius[1] = ceil(roiRadius[1]);
       newROIRadius[2] = ceil(roiRadius[2]);
       std::cout << "ROI XYZ: " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << std::endl;
       roiNode->SetXYZ(newROIXYZ);
       roiNode->SetRadiusXYZ(newROIRadius);
       MRMLUpdateROIFromROINode();
       this->ROIMapUpdate();
       // if(this->Render_Filter->GetSize()) this->UpdateROIRender();
       roiUpdateGuard = false;
 
       vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
       app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(newROIXYZ[0], newROIXYZ[1], newROIXYZ[2]);
 //      cerr << "Resetting focal point to " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << endl;
       }
 }

//----------------------------------------------------------------------------
// Propagate ROI changes in EMSegmentRunSegmentationStep MRML to ROINode MRML
void vtkEMSegmentRunSegmentationStep::MRMLUpdateROIFromROINode()
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);
   
  if(!volumeNode)
    return;

  // update roi to correspond to ROI widget
  double *roiXYZ = roiNode->GetXYZ();
  double *roiRadiusXYZ = roiNode->GetRadiusXYZ();

  double bbox0ras[4], bbox1ras[4];
  double bbox0ijk[4], bbox1ijk[4];

  // ROI bounding box in RAS coordinates
  bbox0ras[0] = roiXYZ[0]-roiRadiusXYZ[0];
  bbox0ras[1] = roiXYZ[1]-roiRadiusXYZ[1];
  bbox0ras[2] = roiXYZ[2]-roiRadiusXYZ[2];
  bbox0ras[3] = 1.;
  bbox1ras[0] = roiXYZ[0]+roiRadiusXYZ[0];
  bbox1ras[1] = roiXYZ[1]+roiRadiusXYZ[1];
  bbox1ras[2] = roiXYZ[2]+roiRadiusXYZ[2];
  bbox1ras[3] = 1.;

  vtkMatrix4x4 *rasToijk = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(rasToijk);
  rasToijk->MultiplyPoint(bbox0ras,bbox0ijk);
  rasToijk->MultiplyPoint(bbox1ras,bbox1ijk);
  rasToijk->Delete();
  for(int i=0;i<3;i++)
    {
    double tmp;
    if(bbox0ijk[i]>bbox1ijk[i])
      {
      tmp = bbox0ijk[i];
      bbox0ijk[i] = bbox1ijk[i];
      bbox1ijk[i] = tmp;
      }
    }
  {
    // Has to be set to -1 bc segmentationboundarymin/max starts at 1 not 0 !   
    int pointMin[3] = {bbox0ijk[0] + 1.0, bbox0ijk[1] + 1.0, bbox0ijk[2] + 1.0}; 
    int pointMax[3] = {bbox1ijk[0] + 1.0, bbox1ijk[1] + 1.0, bbox1ijk[2] + 1.0};
   
    if (this->ROICheck(pointMin, pointMax)) 
      {     
    mrmlManager->SetSegmentationBoundaryMin(pointMin);
    mrmlManager->SetSegmentationBoundaryMax(pointMax);
      }
  }
  
}

//----------------------------------------------------------------------------
// Propagate changes in ROINode MRML to EMSegmentRunSegmentationStep ROI MRML
void vtkEMSegmentRunSegmentationStep::MRMLUpdateROINodeFromROI()
{
  double ROIMinIJK[4], ROIMaxIJK[4], ROIMinRAS[4], ROIMaxRAS[4];
  double radius[3], center[3];

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);

  if(!volumeNode) return;


  this->GetGUI()->GetLogic()->DefineValidSegmentationBoundary();
  int minPoint[3];
  int maxPoint[3];
  mrmlManager->GetSegmentationBoundaryMin(minPoint);
  mrmlManager->GetSegmentationBoundaryMax(maxPoint);

  //cout << "SegmentationBoundaryMin: " << minPoint[0] << " " << minPoint[1] << " " << minPoint[2] << endl;
  //cout << "SegmentationBoundaryMax: " << maxPoint[0] << " " << maxPoint[1] << " " << maxPoint[2] << endl;

  // Has to be set to -1 bc segmentationboundarymin/max starts at 1 not 0 !   
  ROIMinIJK[0] = minPoint[0] - 1;
  ROIMinIJK[1] = minPoint[1] -1 ;
  ROIMinIJK[2] = minPoint[2] -1 ;
  ROIMinIJK[3] = 1.;
  ROIMaxIJK[0] = maxPoint[0] -1 ;
  ROIMaxIJK[1] = maxPoint[1] -1 ;
  ROIMaxIJK[2] = maxPoint[2] -1 ;
  ROIMaxIJK[3] = 1.;

  vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToras);
  ijkToras->MultiplyPoint(ROIMinIJK,ROIMinRAS);
  ijkToras->MultiplyPoint(ROIMaxIJK,ROIMaxRAS);
  ijkToras->Delete();

  center[0] = (ROIMaxRAS[0]+ROIMinRAS[0])/2.;
  center[1] = (ROIMaxRAS[1]+ROIMinRAS[1])/2.;
  center[2] = (ROIMaxRAS[2]+ROIMinRAS[2])/2.;

  radius[0] = fabs(ROIMaxRAS[0]-ROIMinRAS[0])/2.;
  radius[1] = fabs(ROIMaxRAS[1]-ROIMinRAS[1])/2.;
  radius[2] = fabs(ROIMaxRAS[2]-ROIMinRAS[2])/2.;

  roiNode->SetXYZ(center[0], center[1], center[2]);
  roiNode->SetRadiusXYZ(radius[0], radius[1], radius[2]);
  roiNode->Modified();
}

// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::TransitionCallback() 
// {
//   // cout << "vtkEMSegmentRunSegmentationStep::TransitionCallback() Start" << endl; 
//   double *roiXYZ, *roiRadius;
//   roiXYZ = roiNode->GetXYZ();
//   roiRadius = roiNode->GetRadiusXYZ();
// //  std::cerr << "Final ROI center: " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << std::endl;
// //  std::cerr << "Final ROI radius: " << roiRadius[0] << ", " << roiRadius[1] << ", " << roiRadius[2] << std::endl;
//   if (this->ROICheck()) { 
//     vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
//     if (!mrmlManager) return;
// 
//     // update the node with the advanced settings
//     vtkKWRadioButton *rc0 = this->ResamplingChoice->GetWidget()->GetWidget(0);
//     vtkKWRadioButton *rc1 = this->ResamplingChoice->GetWidget()->GetWidget(1);
//     vtkKWRadioButton *rc2 = this->ResamplingChoice->GetWidget()->GetWidget(2);
//     if(rc0->GetSelectedState())
//       {
// Kiliam
//       Node->SetResampleChoice(RESCHOICE_NONE);
//       }
//     else if(rc1->GetSelectedState())
//       {
//       Node->SetResampleChoice(RESCHOICE_LEGACY);
//       }
//     else if(rc2->GetSelectedState())
//       {
//       Node->SetResampleChoice(RESCHOICE_ISO);
//       }
//     Node->SetResampleConst(this->SpinResampleConst->GetWidget()->GetValue());
// 
//     // ----------------------------
//     // Create SuperSampledVolume 
//     vtkSlicerApplication *application   = vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
//     vtkMRMLScalarVolumeNode *outputNode = this->GetGUI()->GetLogic()->CreateSuperSample(1);
//     this->GetGUI()->GetLogic()->SaveVolume(application,outputNode); 
// 
//     if (outputNode) {
//        // Prepare to update mrml node with results 
//               
//        // Delete old attached node first 
//        this->GetGUI()->GetLogic()->DeleteSuperSample(1);
// 
//        // Update node 
//        
//        Node->SetScan1_SuperSampleRef(outputNode->GetID());
//        //cout << "==============================" << endl;
//        //cout << "vtkEMSegmentRunSegmentationStep::TransitionCallback " << Node->GetScan1_SuperSampleRef() << " " <<  Node->GetScan1_Ref() << endl;
//        //cout << "==============================" << endl;
// 
//        // Remove blue ROI screen 
//        this->ROIMapRemove();
//        
//        // remove the ROI widget
//        if (roiNode)
//          roiNode->SetVisibility(0);
//        ResetROIRender();
// 
// //       std::cout << "Final ROI in IJK:  X " << Node->GetROIMin(0) << "," << Node->GetROIMax(0) 
// //            << "                   Y " << Node->GetROIMin(1) << "," << Node->GetROIMax(1) 
// //            << "                   Z " << Node->GetROIMin(2) << "," << Node->GetROIMax(2) << std::endl;
// 
// 
//        this->GUI->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
//      } else {
//        vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
//                                         this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
//                                         "Change Tracker", 
//                                         "Could not proceed to next step - scan1 might have disappeared", 
//                                         vtkKWMessageDialog::ErrorIcon); 
//      }
//      // ---------------------------------
//    } else {     
//      vtkKWMessageDialog::PopupMessage(this->GUI->GetApplication(), 
//                                       this->GUI->GetApplicationGUI()->GetMainSlicerWindow(),
//                                       "Change Tracker", 
//                                       "Please define VOI correctly before proceeding", 
//                                       vtkKWMessageDialog::ErrorIcon);
//    }
//   
// }


//----------------------------------------------------------------------------
//void  vtkEMSegmentRunSegmentationStep::HideUserInterface()
//{
//  this->Superclass::HideUserInterface();
//  this->RemoveROISamplingGUIObservers();
//}

void vtkEMSegmentRunSegmentationStep::ROIIntensityMinMaxUpdate(vtkImageData* image, double &intensityMin, double &intensityMax)
{
  if(!this->ROICheck())
  {
    return;
  }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager) return;

  int minPoint[3];
  mrmlManager->GetSegmentationBoundaryMin(minPoint);
  int maxPoint[3];
  mrmlManager->GetSegmentationBoundaryMax(maxPoint);


  if(maxPoint[0]-minPoint[0]<=0 ||
     maxPoint[1]-minPoint[1]<=0 ||
     maxPoint[2]-minPoint[2]<=0)
    {
    return;
    }

  int ijk[3], ijkMin[3], ijkMax[3], ijkInc[3], i;
  
  for(i=0;i<3;i++)
    {
      // Segmentationboundary starts at 1 where slicer IJK at 0 !
    ijkMax[i] = maxPoint[i]-1;
    ijkMin[i] = minPoint[i]-1 ;
    
    ijkInc[i] = (ijkMax[i]-ijkMin[i])/10;
    if(ijkInc[i]<=0)
      ijkInc[i] = 1;
    }

  intensityMin = image->GetScalarComponentAsDouble(ijkMin[0],ijkMin[1],ijkMin[2],0);
  intensityMax = image->GetScalarComponentAsDouble(ijkMin[0],ijkMin[1],ijkMin[2],0);

  for(ijk[0]=ijkMin[0];ijk[0]<ijkMax[0];ijk[0]+=ijkInc[0])
    {
    for(ijk[1]=ijkMin[1];ijk[1]<ijkMax[1];ijk[1]+=ijkInc[1])
      {
      for(ijk[2]=ijkMin[2];ijk[2]<ijkMax[2];ijk[2]+=ijkInc[2])
        {
        double intensity =
          image->GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],0);
        if(intensityMin>intensity)
          intensityMin = intensity;
        if(intensityMax<intensity)
          intensityMax = intensity;
        }
      }
    }
}

//----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::InitROIRender()
// { 
//   if(this->roiNode)
//     {
//     vtkMRMLEMSegmentRunSegmentationStepNode* Node = this->GetGUI()->GetNode();
//     vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
//     vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode();
//     if(volumeNode)
//       {
//       CreateRender(volumeNode, 0);
//       }
//     }
// }
// 
// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::UpdateROIRender()
// {
//   vtkMRMLEMSegmentRunSegmentationStepNode* node = this->GetGUI()->GetNode();
//   vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
//   vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode();
// 
//   if(volumeNode)
//     {
//     float color0[3] = { 0.8, 0.8, 0.0 };
//     float color1[3] = { 0.8, 0.8, 0.0 };
//     double intensityMin, intensityMax;
//     
//     double* imgRange  =   volumeNode->GetImageData()->GetPointData()->GetScalars()->GetRange();
//     intensityMin = imgRange[0];
//     intensityMax = imgRange[1];
//     this->ROIIntensityMinMaxUpdate(volumeNode->GetImageData(), intensityMin, intensityMax);
//     this->SetRender_BandPassFilter((intensityMax+intensityMin)*.4, intensityMax-1, color0, color1);
// 
//     if(this->Render_RayCast_Mapper)
//       {
//       this->Render_Mapper->SetCroppingRegionPlanes(node->GetROIMin(0), node->GetROIMax(0),
//                                                    node->GetROIMin(1), node->GetROIMax(1),
//                                                    node->GetROIMin(2), node->GetROIMax(2));
//       this->Render_Mapper->CroppingOn();
//       this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
//       }
// 
//     if(this->Render_Mapper)
//       {
//       // !!!  Cropping region is defined in voxel coordinates !!!
//       this->Render_Mapper->SetCroppingRegionPlanes(node->GetROIMin(0), node->GetROIMax(0),
//                                                    node->GetROIMin(1), node->GetROIMax(1),
//                                                    node->GetROIMin(2), node->GetROIMax(2));
//         
//       this->Render_Mapper->CroppingOn();
//       this->GetGUI()->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
//       }
//     }
// }
// 
// //----------------------------------------------------------------------------
// void vtkEMSegmentRunSegmentationStep::ResetROIRender()
// {
//   this->Render_Filter->RemoveAllPoints();
// }
// 
//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ResetROICenter(int *center)
{
  double pointRAS[4], pointIJK[4];
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLVolumeNode* volumeNode =  mrmlManager->GetWorkingDataNode()->GetInputTargetNode()->GetNthVolumeNode(0);

  if(!volumeNode)
    return;
  vtkMatrix4x4 *ijkToras = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToras);
  pointIJK[0] = (double)center[0];
  pointIJK[1] = (double)center[1];
  pointIJK[2] = (double)center[2];
  pointIJK[3] = 1.;
  ijkToras->MultiplyPoint(pointIJK,pointRAS);
  ijkToras->Delete();

  roiNode->SetXYZ(pointRAS[0], pointRAS[1], pointRAS[2]);

  this->CenterRYGSliceViews(pointRAS[0], pointRAS[1], pointRAS[2]);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::ROIMRMLCallback(vtkObject *caller,
                                              unsigned long event,
                                              void *clientData,
                                              void *vtkNotUsed(callData))
{
  vtkEMSegmentRunSegmentationStep *thisStep = reinterpret_cast<vtkEMSegmentRunSegmentationStep*>(clientData);

  vtkMRMLROINode *roiCaller = vtkMRMLROINode::SafeDownCast(caller);
  if(roiCaller && roiCaller == thisStep->roiNode && event == vtkCommand::ModifiedEvent && !thisStep->roiUpdateGuard)
    {
//    vtkMRMLEMSegmentRunSegmentationStepNode* node = thisStep->GetGUI()->GetNode();

    thisStep->roiUpdateGuard = true;

    thisStep->MRMLUpdateROIFromROINode();

//    thisStep->ROIX->SetRange(node->GetROIMin(0), node->GetROIMax(0));
//    thisStep->ROIY->SetRange(node->GetROIMin(1), node->GetROIMax(1));
//    thisStep->ROIZ->SetRange(node->GetROIMin(2), node->GetROIMax(2));

    thisStep->ROIMapUpdate();
    // if(thisStep->Render_Filter->GetSize()) thisStep->UpdateROIRender();
    thisStep->roiUpdateGuard = false;

    double *roiXYZ = thisStep->roiNode->GetXYZ();
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(thisStep->GetGUI()->GetApplication());
    app->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(roiXYZ[0], roiXYZ[1], roiXYZ[2]);
    //      cerr << "Resetting focal point to " << roiXYZ[0] << ", " << roiXYZ[1] << ", " << roiXYZ[2] << endl;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRunSegmentationStep::SetRedGreenYellowAllVolumes(vtkSlicerApplicationGUI *applicationGUI, const char* VolID)
{
  vtkSlicerSliceGUI *redGUI, *greenGUI, *yellowGUI;
  double oldSliceSetting[3];

  redGUI = applicationGUI->GetMainSliceGUI("Red");
  greenGUI = applicationGUI->GetMainSliceGUI("Green");
  yellowGUI = applicationGUI->GetMainSliceGUI("Yellow");

  oldSliceSetting[0] = double(redGUI->GetLogic()->GetSliceOffset());
  oldSliceSetting[1] = double(greenGUI->GetLogic()->GetSliceOffset());
  oldSliceSetting[2] = double(yellowGUI->GetLogic()->GetSliceOffset());

  redGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(VolID);
  redGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");
  yellowGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(VolID);
  yellowGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");
  greenGUI->GetLogic()->GetSliceCompositeNode()->SetBackgroundVolumeID(VolID);
  greenGUI->GetLogic()->GetSliceCompositeNode()->SetForegroundVolumeID("");

  redGUI->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
  greenGUI->GetLogic()->SetSliceOffset(oldSliceSetting[1]);
  yellowGUI->GetLogic()->SetSliceOffset(oldSliceSetting[2]);
}

void vtkEMSegmentRunSegmentationStep::CenterRYGSliceViews(double ptX, double ptY, double ptZ) 
{
      vtkSlicerApplicationGUI *applicationGUI     = this->GetGUI()->GetApplicationGUI();
      applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->SetValue(ptZ);
      applicationGUI->GetMainSliceGUI("Yellow")->GetSliceController()->GetOffsetScale()->SetValue(ptX);
      applicationGUI->GetMainSliceGUI("Green")->GetSliceController()->GetOffsetScale()->SetValue(ptY);
}

  // Create the working directory frame

 

  // Create the save intermediate results button

 

  // Create the generate surface model button
  // if (!this->RunSegmentationGenerateSurfaceCheckButton)
  //   {
  //   this->RunSegmentationGenerateSurfaceCheckButton = 
  //     vtkKWCheckButtonWithLabel::New();
  //   }
  // if (!this->RunSegmentationGenerateSurfaceCheckButton->IsCreated())
  //   {
  //   this->RunSegmentationGenerateSurfaceCheckButton->SetParent(this->RunSegmentationDirectorySubFrame);
  //   this->RunSegmentationGenerateSurfaceCheckButton->Create();
  //   this->RunSegmentationGenerateSurfaceCheckButton->GetLabel()->
  //     SetWidth(EMSEG_WIDGETS_LABEL_WIDTH - 6);
  //   this->RunSegmentationGenerateSurfaceCheckButton->
  //     SetLabelText("Generate Surface Model:");
  //   this->RunSegmentationGenerateSurfaceCheckButton->
  //     GetWidget()->SetCommand(this, "GenerateSurfaceModelsCallback");
  //   }

  // disable for now, not used and confusing
  //   this->Script(
  //     "pack %s -side top -anchor nw -padx 2 -pady 2", 
  //     this->RunSegmentationGenerateSurfaceCheckButton->GetWidgetName());
  
  //this->RunSegmentationGenerateSurfaceCheckButton->GetWidget()->SetSelectedState(mrmlManager->GetSaveSurfaceModels());
  //this->RunSegmentationGenerateSurfaceCheckButton->SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);


  
  // Create the output image frame

  // if (!this->RunSegmentationOutputFrame)
  //   {
  //   this->RunSegmentationOutputFrame = vtkKWFrameWithLabel::New();
  //   }
  // if (!this->RunSegmentationOutputFrame->IsCreated())
  //   {
  //   this->RunSegmentationOutputFrame->SetParent(parent);
  //   this->RunSegmentationOutputFrame->Create();
  //   this->RunSegmentationOutputFrame->SetLabelText("Output Labelmap");
  //   }
  // this->Script(
  //   "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
  //   this->RunSegmentationOutputFrame->GetWidgetName());
  // 
  // // Create the output image frame
  // 
  // if (!this->RunSegmentationOutVolumeSelector)
  //   {
  //   this->RunSegmentationOutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  //   }
  // if (!this->RunSegmentationOutVolumeSelector->IsCreated())
  //   {
  //   this->RunSegmentationOutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",  "LabelMap", "1",  "EMSegment");
  //   this->RunSegmentationOutVolumeSelector->SetNewNodeEnabled(1);
  //   this->RunSegmentationOutVolumeSelector->SetParent(this->RunSegmentationOutputFrame->GetFrame());
  //   this->RunSegmentationOutVolumeSelector->Create();
  //   this->RunSegmentationOutVolumeSelector->SetMRMLScene(mrmlManager->GetMRMLScene());
  // 
  //   this->RunSegmentationOutVolumeSelector->SetBorderWidth(2);
  //   this->RunSegmentationOutVolumeSelector->SetLabelText( "Output Label Map: ");
  //   this->RunSegmentationOutVolumeSelector->SetBalloonHelpString("select an output label map from the current mrml scene.");
  //   }
  // 
  // this->RunSegmentationOutVolumeSelector->UpdateMenu();
  // //if(!mrmlManager->GetOutputVolumeNode())
  // //  {
  // //    this->RunSegmentationOutVolumeSelector->ProcessNewNodeCommand("vtkMRMLScalarVolumeNode", "EM Map");
  // //  }
  // 
  // // cout << "Trying to set selcted mrmlManager->GetOutputVolumeMRMLID())
  // this->RunSegmentationOutVolumeSelector->SetSelected(mrmlManager->GetMRMLScene()->GetNodeByID(mrmlManager->GetOutputVolumeMRMLID()));
  // 
  // this->RunSegmentationOutVolumeSelector->SetEnabled(
  //   mrmlManager->HasGlobalParametersNode() ? enabled : 0);
  // 
  // this->Script(
  //   "pack %s -side top -anchor nw -padx 2 -pady 2", 
  //   this->RunSegmentationOutVolumeSelector->GetWidgetName());
  // this->AddRunRegistrationOutputGUIObservers();
  
  // Create the boundary frame
