#include "vtkProstateNavConfigurationStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWCheckButton.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavConfigurationStep);
vtkCxxRevisionMacro(vtkProstateNavConfigurationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavConfigurationStep::vtkProstateNavConfigurationStep()
{
  this->SetName("1/5. Configuration");
  this->SetDescription("Perform system configuration.");

  this->ConfigNTFrame = NULL;
  this->ConnectNTFrame = NULL;
  this->LoadConfigButtonNT = NULL;
  this->ConfigFileEntryNT = NULL;
  this->ConnectCheckButtonNT = NULL;
}

//----------------------------------------------------------------------------
vtkProstateNavConfigurationStep::~vtkProstateNavConfigurationStep()
{
  if (this->LoadConfigButtonNT)
    {
    this->LoadConfigButtonNT->GetWidget()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->LoadConfigButtonNT->SetParent(NULL );
    this->LoadConfigButtonNT->Delete();
    }
  if (this->ConfigFileEntryNT)
    {
    this->ConfigFileEntryNT->SetParent(NULL );
    this->ConfigFileEntryNT->Delete();
    }
  if (this->ConnectNTFrame)
    {
    this->ConnectNTFrame->SetParent(NULL);
    this->ConnectNTFrame->Delete();
    }
  if (this->ConfigNTFrame)
    {
    this->ConfigNTFrame->SetParent(NULL);
    this->ConfigNTFrame->Delete();
    }

  if (this->ConnectCheckButtonNT)
    {
    this->ConnectCheckButtonNT->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->ConnectCheckButtonNT->SetParent(NULL);
    this->ConnectCheckButtonNT->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkProstateNavConfigurationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // Create the frame
  if (!this->ConfigNTFrame)
    {
    this->ConfigNTFrame = vtkKWFrame::New();
    this->ConfigNTFrame->SetParent ( parent );
    this->ConfigNTFrame->Create ( );
    }
  
  if (!this->ConnectNTFrame)
    {
    this->ConnectNTFrame = vtkKWFrame::New();
    this->ConnectNTFrame->SetParent ( parent );
    this->ConnectNTFrame->Create ( );
    }

  if (!this->LoadConfigButtonNT)
    {
    this->LoadConfigButtonNT = vtkKWLoadSaveButtonWithLabel::New();
    this->LoadConfigButtonNT->SetParent(this->ConfigNTFrame);
    this->LoadConfigButtonNT->Create();
    //this->LoadConfigButtonNT->SetWidth(30);
    this->LoadConfigButtonNT->SetLabelText("Config. File:");
    this->LoadConfigButtonNT->GetWidget()->SetText ("Browse Config File");
    this->LoadConfigButtonNT->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
      "{ {ProstateNav} {*.xml} }");
    this->LoadConfigButtonNT->GetWidget()->GetLoadSaveDialog()
      ->RetrieveLastPathFromRegistry("OpenPath");

    this->LoadConfigButtonNT->GetWidget()
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }

  // Create the file entry and load button
  if (!this->ConfigFileEntryNT)
    {
    this->ConfigFileEntryNT = vtkKWEntry::New();
    this->ConfigFileEntryNT->SetParent(this->ConfigNTFrame);
    this->ConfigFileEntryNT->Create();
    this->ConfigFileEntryNT->SetWidth(50);
    this->ConfigFileEntryNT->SetValue ("");
    
    }

  // The connnect button 
  if (!this->ConnectCheckButtonNT)
    {
    this->ConnectCheckButtonNT = vtkKWCheckButton::New();
    this->ConnectCheckButtonNT->SetParent(this->ConnectNTFrame);
    this->ConnectCheckButtonNT->Create();
    this->ConnectCheckButtonNT->SelectedStateOff();
    this->ConnectCheckButtonNT->SetText("Connect");

    this->ConnectCheckButtonNT
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

    }


  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->ConfigNTFrame->GetWidgetName());
    
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->ConnectNTFrame->GetWidgetName());


  this->Script("pack %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->LoadConfigButtonNT->GetWidgetName());

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->ConnectCheckButtonNT->GetWidgetName());

}

//----------------------------------------------------------------------------
void vtkProstateNavConfigurationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavConfigurationStep::ProcessGUIEvents( vtkObject *caller,
                                         unsigned long event, void *callData )
{

  if (this->LoadConfigButtonNT->GetWidget() == vtkKWLoadSaveButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent )
    {
    const char* filename = this->LoadConfigButtonNT->GetWidget()->GetFileName();
    if (filename)
      {
      const vtksys_stl::string fname(filename);
      this->ConfigFileEntryNT->SetValue(fname.c_str());
      }
    else
      {
      this->ConfigFileEntryNT->SetValue("");
      }
    }

  else if (this->ConnectCheckButtonNT == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {

    if (this->ConnectCheckButtonNT->GetSelectedState() && this->Logic)
      {
      // Activate NaviTrack Stream
      const char* filename = this->LoadConfigButtonNT->GetWidget()->GetFileName();
      this->Logic->ConnectTracker(filename);
      }
    else
      {
      // Deactivate NaviTrack Stream
      this->Logic->DisconnectTracker();
      }
    }

}



