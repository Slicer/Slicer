#include "vtkSlicerWindow.h"
#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWindow );
vtkCxxRevisionMacro(vtkSlicerWindow, "$Revision: 1.0 $");

vtkSlicerWindow::vtkSlicerWindow()
{
  this->FeedbackMenu = NULL;
}


vtkSlicerWindow::~vtkSlicerWindow()
{
  if (this->ApplicationSettingsInterface)
    {
    this->ApplicationSettingsInterface->Delete(); 
    this->ApplicationSettingsInterface = NULL;
    }
  if (this->FeedbackMenu)
    {
    this->FeedbackMenu->Delete();
    this->FeedbackMenu = NULL;
    }
}


//----------------------------------------------------------------------------
vtkKWApplicationSettingsInterface* 
vtkSlicerWindow::GetApplicationSettingsInterface()
{
  // If not created, create the application settings interface, connect it
  // to the current window, and manage it with the current interface manager.

  if (!this->ApplicationSettingsInterface)
    {
    this->ApplicationSettingsInterface = 
      vtkSlicerApplicationSettingsInterface::New();
    this->ApplicationSettingsInterface->SetWindow(this);
    this->ApplicationSettingsInterface->SetUserInterfaceManager(
      this->GetApplicationSettingsUserInterfaceManager());
    }
  return this->ApplicationSettingsInterface;
}

//----------------------------------------------------------------------------
void vtkSlicerWindow::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  vtkKWApplication *app = this->GetApplication();
  if (!this->FeedbackMenu)
    {
    this->FeedbackMenu = vtkKWMenu::New();
    }

  if (!this->FeedbackMenu->IsCreated() && this->GetMenu() && this->IsCreated())
    {
    this->FeedbackMenu->SetParent(this->GetMenu());
    this->FeedbackMenu->SetTearOff(0);
    this->FeedbackMenu->Create();
    // Usually at the end
    this->GetMenu()->AddCascade("Feedback", this->FeedbackMenu);
    }
}


//----------------------------------------------------------------------------
void vtkSlicerWindow::UpdateMenuState()
{
  this->Superclass::UpdateMenuState();
  this->PropagateEnableState(this->FeedbackMenu);
}


