#pragma once

// Export
#include "vtkSlicerModelsModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLNode.h"

/// \brief Node for storing picking state and parameters for models.
class VTK_SLICER_MODELS_MODULE_MRML_EXPORT vtkMRMLModelPickingNode : public vtkMRMLNode
{
public:
  static vtkMRMLModelPickingNode* New();
  vtkTypeMacro(vtkMRMLModelPickingNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  const char* GetNodeTagName() override { return "ModelPicking"; }

  /// \brief Picked model node ID
  vtkGetStringMacro(PickedNodeID);
  vtkSetStringMacro(PickedNodeID);

  /// \brief Picked RAS coordinates
  vtkGetVector3Macro(PickedRAS, double);
  vtkSetVector3Macro(PickedRAS, double);

  /// \brief Picked cell ID
  vtkGetMacro(PickedCellID, vtkIdType);
  vtkSetMacro(PickedCellID, vtkIdType);

  /// \brief Picked point ID
  vtkGetMacro(PickedPointID, vtkIdType);
  vtkSetMacro(PickedPointID, vtkIdType);

  /// \brief Picking tolerance
  vtkGetMacro(PickTolerance, double);
  vtkSetMacro(PickTolerance, double);

  /// \brief Enable/disable update of the picking node
  vtkGetMacro(Enabled, bool);
  vtkSetMacro(Enabled, bool);
  vtkBooleanMacro(Enabled, bool);

  /// \brief View ID from where the pick is coming from
  vtkGetStringMacro(ViewID);
  vtkSetStringMacro(ViewID);

protected:
  vtkMRMLModelPickingNode();
  ~vtkMRMLModelPickingNode() override;
  vtkMRMLModelPickingNode(const vtkMRMLModelPickingNode&) = delete;
  void operator=(const vtkMRMLModelPickingNode&) = delete;

  char* PickedNodeID;
  char* ViewID;
  double PickedRAS[3];
  vtkIdType PickedCellID;
  vtkIdType PickedPointID;
  double PickTolerance;
  bool Enabled;
};
