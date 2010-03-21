#include "vtkLiverAblationPlanningStep.h"

#include "vtkLiverAblationGUI.h"
#include "vtkLiverAblationMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMessageDialog.h"

#include "vtkKWGuideWidget.h"
#include "vtkKWInternationalization.h"

#include "vtkSlicerApplication.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkLiverAblationPlanningStep);
vtkCxxRevisionMacro(vtkLiverAblationPlanningStep, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkLiverAblationPlanningStep::vtkLiverAblationPlanningStep()
{
  this->SetName("2/4. Planning");
  this->SetDescription("Use existing Slicer modules to perform surgical planning.");
  this->PreoperativeImageDataMenuButton = NULL;
  this->ToolModelMenuButton = NULL;

  this->AddVolumeButton = NULL; 
  this->GuideWidgetButton = NULL; 

  this->GuideWidget  = NULL;
}

//----------------------------------------------------------------------------
vtkLiverAblationPlanningStep::~vtkLiverAblationPlanningStep()
{  
  if(this->PreoperativeImageDataMenuButton)
    {
    this->PreoperativeImageDataMenuButton->Delete();
    this->PreoperativeImageDataMenuButton = NULL;
    }

  if(this->ToolModelMenuButton)
    {
    this->ToolModelMenuButton->Delete();
    this->ToolModelMenuButton = NULL;
    }

  if(this->AddVolumeButton)
    {
    this->AddVolumeButton->Delete();
    this->AddVolumeButton = NULL;
    }

  if(this->GuideWidgetButton)
    {
    this->GuideWidgetButton->Delete();
    this->GuideWidgetButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  if (!this->GuideWidgetButton)
    {
    this->GuideWidgetButton = vtkKWPushButton::New();
    this->GuideWidgetButton->SetParent (parent);
    this->GuideWidgetButton->Create();
    this->GuideWidgetButton->SetText("Show Guide Widget");
    this->GuideWidgetButton->SetCommand(this, "GuideWidgetButtonCallback");
    this->GuideWidgetButton->SetWidth(25);
    }

  if (!this->AddVolumeButton)
    {
    this->AddVolumeButton = vtkKWPushButton::New();
    this->AddVolumeButton->SetParent (parent);
    this->AddVolumeButton->Create();
    this->AddVolumeButton->SetText("Add a volume");
    this->AddVolumeButton->SetCommand(this, "AddVolumeButtonCallback");
    this->AddVolumeButton->SetWidth(25);
    }

  this->Script(
    "pack %s %s -side top -anchor center -padx 2 -pady 5", 
    this->AddVolumeButton->GetWidgetName(),
    this->GuideWidgetButton->GetWidgetName());

/* 
  // Create the preoperative image data  menu button

  if (!this->PreoperativeImageDataMenuButton)
    {
    this->PreoperativeImageDataMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->PreoperativeImageDataMenuButton->IsCreated())
    {
    this->PreoperativeImageDataMenuButton->SetParent(parent);
    this->PreoperativeImageDataMenuButton->Create();
    this->PreoperativeImageDataMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->PreoperativeImageDataMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10);
    this->PreoperativeImageDataMenuButton->
      SetLabelText("Preoperative Image:");
    this->PreoperativeImageDataMenuButton->
      SetBalloonHelpString("Select preoperative image data.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 5", 
    this->PreoperativeImageDataMenuButton->GetWidgetName());
  
  this->PopulatePreoperativeImageDataSelector();

  // Create the calibration data menu button

  if (!this->ToolModelMenuButton)
    {
    this->ToolModelMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ToolModelMenuButton->IsCreated())
    {
    this->ToolModelMenuButton->SetParent(parent);
    this->ToolModelMenuButton->Create();
    this->ToolModelMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH+10);
    this->ToolModelMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH-10); 
    this->ToolModelMenuButton->
      SetLabelText("Model data :");
    this->ToolModelMenuButton->
      SetBalloonHelpString("Select model(mesh) data for surgical tool or endoscope probe .");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 5", 
    this->ToolModelMenuButton->GetWidgetName());
  
  this->PopulateToolModelSelector();
*/

   //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Load Preoperative Images Step");
  msg_dlg1->SetText( "This is the first step in IGT applications. In this step, the user loads "
                     "preoperative data required for IGT application. This includes images "
                     "(CT, MRI data), surgical tool/endoscope probe models, and surgical planning "
                     "information");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}
 
//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::PopulateToolModelSelector()
{
  char buffer[256];

  vtkLiverAblationMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkKWMenu* menu = this->ToolModelMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  int nb_of_models = mrmlManager->GetModelNumberOfChoices();
  for(int index=0; index < nb_of_models; index++)
    {
    vtkIdType vol_id = mrmlManager->GetModelNthID(index);
    sprintf(buffer, "%d", static_cast<int>(vol_id));
    const char *name = mrmlManager->GetModelName(vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::PopulatePreoperativeImageDataSelector()
{
  char buffer[256];

  vtkLiverAblationMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkKWMenu* menu = this->PreoperativeImageDataMenuButton->
    GetWidget()->GetMenu();
  menu->DeleteAllItems();

  int nb_of_volumes = mrmlManager->GetVolumeNumberOfChoices();
  for(int index=0; index < nb_of_volumes; index++)
    {
    vtkIdType vol_id = mrmlManager->GetVolumeNthID(index);
    sprintf(buffer, "%d", static_cast<int>(vol_id));
    const char *name = mrmlManager->GetVolumeName(vol_id);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}
 
//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{
  // AddVolumeButton Pressed

  if (this->AddVolumeButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    this->GetGUI()->GetApplication()->Script("::LoadVolume::ShowDialog");
//    this->GetGUI()->GetApplication();
    }
}

//----------------------------------------------------------------------------
vtkKWGuideWidget* vtkLiverAblationPlanningStep::GetGuideWidget()
{
  if (!this->GuideWidget)
    {
    //this->GuideWidget = vtkKWTkcon::New();
    this->GuideWidget = vtkKWGuideWidget::New();
    }

  if (!this->GuideWidget->IsCreated())
    {
    this->GuideWidget->SetApplication(this->GetApplication());
    this->GuideWidget->Create();
    this->GuideWidget->SetSize(253, 40);
    this->GuideWidget->SetSlicerAppGUI(this->GetGUI()->GetApplicationGUI());
    }
  
  return this->GuideWidget;

  /*
  if (!this->GuideWidget)
    {
    this->GuideWidget = vtkKWGuideWidget::New();
    this->GetGuideWidget()->SetApplication(this->GetApplication());
    }
  return this->GuideWidget;
  */
}


//----------------------------------------------------------------------------
int vtkLiverAblationPlanningStep::CreateGuideWidget()
{
  if (this->GetGuideWidget())
    {
    if (!this->GetGuideWidget()->IsCreated())
      {
      this->GetGuideWidget()->Create();
      }
    return this->GetGuideWidget()->IsCreated();
    }
  return 0;
}


//----------------------------------------------------------------------------
void vtkLiverAblationPlanningStep::DisplayGuideWidget(vtkKWTopLevel* master)
{
  vtkKWGuideWidget *widget = this->GetGuideWidget();
  if (widget)
    {
    if (!master)
      {
      master = this->GetGUI()->GetApplication()->GetNthWindow(0);
      }

    if (master)
      {
      vtksys_stl::string title;
      title += ks_("IGT Module Guide Widget|Title|IGT Module Guide");
      widget->SetTitle(title.c_str());
      widget->SetMasterWindow(master);
      }
    widget->Display(135, 121);
    }
}


void vtkLiverAblationPlanningStep::AddVolumeButtonCallback()
{
  // AddVolumeButton Pressed

  if (this->AddVolumeButton)
    {
    this->GetGUI()->GetApplication()->Script("::LoadVolume::ShowDialog");
    }
}


void vtkLiverAblationPlanningStep::GuideWidgetButtonCallback()
{
  this->DisplayGuideWidget(NULL);
}

