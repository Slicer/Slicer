#include "vtkBrainlabModuleLoadingDataStep.h"

#include "vtkBrainlabModuleGUI.h"
#include "vtkBrainlabModuleMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWInternationalization.h"

#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleLoadingDataStep);
vtkCxxRevisionMacro(vtkBrainlabModuleLoadingDataStep, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkBrainlabModuleLoadingDataStep::vtkBrainlabModuleLoadingDataStep()
{
  this->SetName("1/3. Load an MRML Scene");
  this->SetDescription("Load user's own scene.");

//  this->DefaultSceneButton = NULL; 
  this->UserSceneButton = NULL; 
}

//----------------------------------------------------------------------------
vtkBrainlabModuleLoadingDataStep::~vtkBrainlabModuleLoadingDataStep()
{  
  /*
  if(this->DefaultSceneButton)
    {
    this->DefaultSceneButton->Delete();
    this->DefaultSceneButton = NULL;
    }
  */
  if(this->UserSceneButton)
    {
    this->UserSceneButton->Delete();
    this->UserSceneButton = NULL;
    }
}



//----------------------------------------------------------------------------
void vtkBrainlabModuleLoadingDataStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  if (!this->UserSceneButton)
    {
    this->UserSceneButton = vtkKWPushButton::New();
    this->UserSceneButton->SetParent (parent);
    this->UserSceneButton->Create();
    this->UserSceneButton->SetText("Load your own scene");
    this->UserSceneButton->SetCommand(this, "UserSceneButtonCallback");
    this->UserSceneButton->SetWidth(25);
    }

  /*
  if (!this->DefaultSceneButton)
    {
    this->DefaultSceneButton = vtkKWPushButton::New();
    this->DefaultSceneButton->SetParent (parent);
    this->DefaultSceneButton->Create();
    this->DefaultSceneButton->SetText("Load default scene");
    this->DefaultSceneButton->SetCommand(this, "DefaultSceneButtonCallback");
    this->DefaultSceneButton->SetWidth(25);
    }
    */

  this->Script(
    "pack %s -side top -anchor center -padx 2 -pady 7", 
    this->UserSceneButton->GetWidgetName());

   //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Load an MRML Scene");
  msg_dlg1->SetText( "This is the first step in BrainlabModule. In this step, the user loads "
                     "an MRML scene, which usually includes a tensor image and a fiducial."
                     "The fiducial is a seed for DTI tractography visualization.");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}
 


//----------------------------------------------------------------------------
void vtkBrainlabModuleLoadingDataStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkBrainlabModuleLoadingDataStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{
  /*
  // DefaultSceneButton Pressed

  if (this->DefaultSceneButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
//    this->GetGUI()->GetApplication()->Script("::LoadVolume::ShowDialog");
    }
  */
}



/*
void vtkBrainlabModuleLoadingDataStep::DefaultSceneButtonCallback()
{
  // DefaultSceneButton Pressed

  if (this->DefaultSceneButton)
    {
//    this->GetGUI()->GetApplication()->Script("::LoadScene::ShowDialog");
    }
}
*/



void vtkBrainlabModuleLoadingDataStep::UserSceneButtonCallback()
{
  // UserSceneButton Pressed

  if (this->UserSceneButton)
    {
    this->GetGUI()->GetApplicationGUI()->ProcessLoadSceneCommand();
    vtkBrainlabModuleGUI *gui = this->GetGUI();
    if (gui)
      {
      gui->GetWizardWidget()->GetWizardWorkflow()->AttemptToGoToNextStep();
      }
    }
}


