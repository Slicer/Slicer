/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#ifndef __vtkSlicerLegacyModuleMacrosTestLogic_h
#define __vtkSlicerLegacyModuleMacrosTestLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerLegacyModuleMacrosTestModuleLogicExport.h"

/// \brief A logic of a module library built with the macros extensions use (SlicerMacroBuildModuleLogic).
///
/// For testing that such a library is wrapped for Python with the base classes from Slicer's
/// libraries (test_slicer_legacy_module_macros.py).
class VTK_SLICER_LEGACYMODULEMACROSTEST_MODULE_LOGIC_EXPORT vtkSlicerLegacyModuleMacrosTestLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerLegacyModuleMacrosTestLogic* New();
  vtkTypeMacro(vtkSlicerLegacyModuleMacrosTestLogic, vtkSlicerModuleLogic);
  /// Overridden as a module logic usually does: a class that overrides a method of its base class is
  /// wrapped as one that cannot be instantiated when the wrapping does not have the base class.
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerLegacyModuleMacrosTestLogic() = default;
  ~vtkSlicerLegacyModuleMacrosTestLogic() override = default;

private:
  vtkSlicerLegacyModuleMacrosTestLogic(const vtkSlicerLegacyModuleMacrosTestLogic&) = delete;
  void operator=(const vtkSlicerLegacyModuleMacrosTestLogic&) = delete;
};

#endif
