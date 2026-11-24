/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#ifndef __vtkMRMLLegacyModuleMacrosTestNode_h
#define __vtkMRMLLegacyModuleMacrosTestNode_h

// MRML includes
#include <vtkMRMLNode.h>

#include "vtkSlicerLegacyModuleMacrosTestModuleMRMLExport.h"

/// \brief A node of a module library built with the macros extensions use (SlicerMacroBuildModuleMRML).
///
/// For testing that such a library is wrapped for Python with the base classes from Slicer's
/// libraries (test_slicer_legacy_module_macros.py).
class VTK_SLICER_LEGACYMODULEMACROSTEST_MODULE_MRML_EXPORT vtkMRMLLegacyModuleMacrosTestNode : public vtkMRMLNode
{
public:
  static vtkMRMLLegacyModuleMacrosTestNode* New();
  vtkTypeMacro(vtkMRMLLegacyModuleMacrosTestNode, vtkMRMLNode);
  /// Overridden as a node usually does: a class that overrides a method of its base class is wrapped
  /// as one that cannot be instantiated when the wrapping does not have the base class.
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "LegacyModuleMacrosTest"; }

protected:
  vtkMRMLLegacyModuleMacrosTestNode() = default;
  ~vtkMRMLLegacyModuleMacrosTestNode() override = default;
  vtkMRMLLegacyModuleMacrosTestNode(const vtkMRMLLegacyModuleMacrosTestNode&);
  void operator=(const vtkMRMLLegacyModuleMacrosTestNode&);
};

#endif
