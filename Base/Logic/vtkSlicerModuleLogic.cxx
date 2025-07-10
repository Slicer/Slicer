/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// SlicerLogic includes
#include "vtkSlicerModuleLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::vtkSlicerModuleLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerModuleLogic::~vtkSlicerModuleLogic()
{
  this->SetMRMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------------
vtkSlicerApplicationLogic* vtkSlicerModuleLogic::GetApplicationLogic()
{
  return vtkSlicerApplicationLogic::SafeDownCast(this->GetMRMLApplicationLogic());
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLogic* vtkSlicerModuleLogic::GetModuleLogic(const char* moduleName)
{
  vtkMRMLApplicationLogic* appLogic = vtkMRMLApplicationLogic::SafeDownCast(this->GetMRMLApplicationLogic());
  if (!appLogic)
  {
    return nullptr;
  }
  return appLogic->GetModuleLogic(moduleName);
}

//----------------------------------------------------------------------------
std::string vtkSlicerModuleLogic::GetModuleShareDirectory() const
{
  return this->ModuleShareDirectory;
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::SetModuleShareDirectory(const std::string& shareDirectory)
{
  if (this->ModuleShareDirectory != shareDirectory)
  {
    this->ModuleShareDirectory = shareDirectory;
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkSlicerModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
