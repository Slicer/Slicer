/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkSlicerModuleLogic - superclass for slicer module logic classes
///
/// Superclass for all slicer module logic classes
/// \note No GUI code goes in the logic class.

#ifndef __vtkSlicerModuleLogic_h
#define __vtkSlicerModuleLogic_h

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRMLLogic includes
#include <vtkMRMLAbstractLogic.h>

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModuleLogic
  : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkSlicerModuleLogic *New();
  vtkTypeMacro(vtkSlicerModuleLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get access to overall application state
  virtual vtkSlicerApplicationLogic* GetApplicationLogic();
  //TODO virtual void SetApplicationLogic(vtkSlicerApplicationLogic* logic);

  std::string GetModuleShareDirectory()const;
  void SetModuleShareDirectory(const std::string& shareDirectory);

protected:

  vtkSlicerModuleLogic();
  ~vtkSlicerModuleLogic() override;

private:

  vtkSlicerModuleLogic(const vtkSlicerModuleLogic&) = delete;
  void operator=(const vtkSlicerModuleLogic&) = delete;

  std::string ModuleShareDirectory;
};

#endif

