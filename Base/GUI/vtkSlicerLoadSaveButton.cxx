#include "vtkSlicerLoadSaveButton.h"

#include "vtkKWIcon.h"
#include "vtkSlicerLoadSaveDialog.h"
#include "vtkObjectFactory.h"

#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerLoadSaveButton);
vtkCxxRevisionMacro(vtkSlicerLoadSaveButton, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkSlicerLoadSaveButton::vtkSlicerLoadSaveButton()
{
  this->LoadSaveDialog = vtkSlicerLoadSaveDialog::New();

  this->MaximumFileNameLength = 30;
  this->TrimPathFromFileName  = 1;

  this->AddCallbackCommandObservers();
}

//----------------------------------------------------------------------------
vtkSlicerLoadSaveButton::~vtkSlicerLoadSaveButton()
{
  if (this->LoadSaveDialog)
    {
    this->LoadSaveDialog->Delete();
    this->LoadSaveDialog = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro("PopupButton already created");
    return;
    }

  // Call the superclass, this will set the application and 
  // create the pushbutton.

  this->Superclass::CreateWidget();

  // Cosmetic add-on

  this->SetImageToPredefinedIcon(vtkKWIcon::IconFolder);
  this->SetCompoundModeToLeft();
  this->SetPadX(3);
  this->SetPadY(2);

  // No filename yet, set it to empty

  if (!this->GetText())
    {
    this->SetText("");
    }

  // Create the load/save dialog

  this->LoadSaveDialog->SetParent(this);
  this->LoadSaveDialog->Create();
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::InvokeCommand()
{
  this->LoadSaveDialog->Invoke();

  this->Superclass::InvokeCommand();
}

//----------------------------------------------------------------------------
const char* vtkSlicerLoadSaveButton::GetFileName()
{
  if (this->LoadSaveDialog)
    {
    return this->LoadSaveDialog->GetFileName();
    }
  return NULL;
} 

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::SetMaximumFileNameLength(int arg)
{
  if (this->MaximumFileNameLength == arg)
    {
    return;
    }

  this->MaximumFileNameLength = arg;
  this->Modified();

  this->UpdateTextFromFileName();
} 

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::SetTrimPathFromFileName(int arg)
{
  if (this->TrimPathFromFileName == arg)
    {
    return;
    }

  this->TrimPathFromFileName = arg;
  this->Modified();

  this->UpdateTextFromFileName();
} 

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::UpdateTextFromFileName()
{
  const char *fname = this->GetFileName();
  if (!fname || !*fname)
    {
    return;
    }

  if (this->MaximumFileNameLength <= 0 && !this->TrimPathFromFileName)
    {
    this->SetText(fname);
    }
  else
    {
    vtksys_stl::string new_fname; 
    if (this->TrimPathFromFileName)
      {
      new_fname = vtksys::SystemTools::GetFilenameName(fname);
      }
    else
      {
      new_fname = fname;
      }
    new_fname = 
      vtksys::SystemTools::CropString(new_fname, this->MaximumFileNameLength);
    this->SetText(new_fname.c_str());
    }
} 

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  this->PropagateEnableState(this->LoadSaveDialog);
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::AddCallbackCommandObservers()
{
  this->Superclass::AddCallbackCommandObservers();

  if (this->LoadSaveDialog)
    {
    this->AddCallbackCommandObserver(
      this->LoadSaveDialog, vtkSlicerLoadSaveDialog::FileNameChangedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::RemoveCallbackCommandObservers()
{
  this->Superclass::RemoveCallbackCommandObservers();

  if (this->LoadSaveDialog)
    {
    this->RemoveCallbackCommandObserver(
      this->LoadSaveDialog, vtkSlicerLoadSaveDialog::FileNameChangedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::ProcessCallbackCommandEvents(vtkObject *caller,
                                                       unsigned long event,
                                                       void *calldata)
{
  if (caller == this->LoadSaveDialog)
    {
    switch (event)
      {
      case vtkSlicerLoadSaveDialog::FileNameChangedEvent:
        this->UpdateTextFromFileName();
        break;
      }
    }

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}

//----------------------------------------------------------------------------
void vtkSlicerLoadSaveButton::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "LoadSaveDialog: " << this->LoadSaveDialog << endl;
  os << indent << "MaximumFileNameLength: " 
     << this->MaximumFileNameLength << endl;
  os << indent << "TrimPathFromFileName: " 
     << (this->TrimPathFromFileName ? "On" : "Off") << endl;
}
