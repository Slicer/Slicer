#include "vtkChangeTrackerGUI.h"
#include "vtkChangeTrackerLogic.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkChangeTrackerSelectScanStep.h"
//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkChangeTrackerSelectScanStep);
vtkCxxRevisionMacro(vtkChangeTrackerSelectScanStep, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkChangeTrackerSelectScanStep::vtkChangeTrackerSelectScanStep()
{
  this->VolumeMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkChangeTrackerSelectScanStep::~vtkChangeTrackerSelectScanStep()
{
  if (this->VolumeMenuButton)
  {
    this->VolumeMenuButton->Delete();
    this->VolumeMenuButton = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSelectScanStep::ShowUserInterface()
{
  this->vtkChangeTrackerStep::ShowUserInterface();

  if (!this->VolumeMenuButton) {
    this->VolumeMenuButton=vtkSlicerNodeSelectorWidget::New();
    this->VolumeMenuButton->SetParent(this->Frame->GetFrame());
    this->VolumeMenuButton->Create();
    this->VolumeMenuButton->SetLabelText("Select Volume");
    this->VolumeMenuButton->NoneEnabledOn();
    this->VolumeMenuButton->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->VolumeMenuButton->SetMRMLScene(this->GetGUI()->GetLogic()->GetMRMLScene());
    this->VolumeMenuButton->GetWidget()->SetWidth(CHANGETRACKER_MENU_BUTTON_WIDTH);
    this->AddGUIObservers();

    // If you want to attach a function once is selected 
    // look at GrayscaleSelector vtkSlicerVolumeMathGUI::AddGUIObservers
  }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSelectScanStep::AddGUIObservers() 
{
  // Make sure you do not add the same event twice - need to do it bc of wizrd structure
  if (this->VolumeMenuButton && (!this->VolumeMenuButton->HasObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand))) 
    {
      this->VolumeMenuButton->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    } 
}

void vtkChangeTrackerSelectScanStep::RemoveGUIObservers() 
{
  if ( this->VolumeMenuButton) 
    {
      this->VolumeMenuButton->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, this->WizardGUICallbackCommand );  
    }
}

void vtkChangeTrackerSelectScanStep::ProcessGUIEvents(vtkObject *caller,
                                                      unsigned long event,
                                                      void *vtkNotUsed(callData)) { 
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (this->VolumeMenuButton && selector == this->VolumeMenuButton && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent && this->VolumeMenuButton->GetSelected() != NULL) 
  { 
    this->GetGUI()->UpdateMRML();
  }
}

//----------------------------------------------------------------------------
void vtkChangeTrackerSelectScanStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
