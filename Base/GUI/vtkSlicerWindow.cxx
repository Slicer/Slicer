#include "vtkSlicerWindow.h"
#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWindow );
vtkCxxRevisionMacro(vtkSlicerWindow, "$Revision: 1.0 $");

vtkSlicerWindow::vtkSlicerWindow()
{
}


vtkSlicerWindow::~vtkSlicerWindow()
{
  if (this->ApplicationSettingsInterface)
    {
    this->ApplicationSettingsInterface->Delete(); 
    this->ApplicationSettingsInterface = NULL;
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

