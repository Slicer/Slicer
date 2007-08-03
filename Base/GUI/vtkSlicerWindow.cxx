#include "vtkSlicerWindow.h"
#include "vtkSlicerApplicationSettingsInterface.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWindow );
vtkCxxRevisionMacro(vtkSlicerWindow, "$Revision: 1.0 $");

vtkSlicerWindow::vtkSlicerWindow()
{
  this->FeedbackMenu = NULL;
  this->FontSizeMenu = NULL;
  this->FontFamilyMenu = NULL;
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
  if ( this->FontSizeMenu )
    {
    this->FontSizeMenu->SetParent ( NULL );
    this->FontSizeMenu->Delete();
    this->FontSizeMenu = NULL;
    }
  if ( this->FontFamilyMenu )
    {
    this->FontFamilyMenu->SetParent ( NULL );
    this->FontFamilyMenu->Delete();
    this->FontFamilyMenu = NULL;
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

  if ( !this->FontSizeMenu )
    {
    this->FontSizeMenu = vtkKWMenu::New();
    }

  if (!this->FontSizeMenu->IsCreated() && this->GetViewMenu() && this->IsCreated())
    {
    this->FontSizeMenu->SetParent ( this->GetViewMenu() );
    this->FontSizeMenu->SetTearOff(0);
    this->FontSizeMenu->Create();
    }

  if ( !this->FontFamilyMenu )
    {
    this->FontFamilyMenu = vtkKWMenu::New();
    }

  if (!this->FontFamilyMenu->IsCreated() && this->GetViewMenu() && this->IsCreated())
    {
    this->FontFamilyMenu->SetParent ( this->GetViewMenu() );
    this->FontFamilyMenu->SetTearOff(0);
    this->FontFamilyMenu->Create();
    }

}


//----------------------------------------------------------------------------
void vtkSlicerWindow::UpdateMenuState()
{
  this->Superclass::UpdateMenuState();
  this->PropagateEnableState(this->FeedbackMenu);
}


