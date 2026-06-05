#include "vtkMRMLModelPickingNode.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLModelPickingNode);

//----------------------------------------------------------------------------
vtkMRMLModelPickingNode::vtkMRMLModelPickingNode()
  : PickedNodeID{ nullptr }
  , ViewID{ nullptr }
  , PickedRAS{ 0.0, 0.0, 0.0 }
  , PickedCellID{ -1 }
  , PickedPointID{ -1 }
  , PickTolerance{ 0.025 } // Default cell picker tolerance
  , Enabled{ true }
{
}

//----------------------------------------------------------------------------
vtkMRMLModelPickingNode::~vtkMRMLModelPickingNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLModelPickingNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(PickedNodeID);
  vtkMRMLPrintVectorMacro(PickedRAS, double, 3);
  vtkMRMLPrintIntMacro(PickedCellID);
  vtkMRMLPrintIntMacro(PickedPointID);
  vtkMRMLPrintFloatMacro(PickTolerance);
  vtkMRMLPrintBooleanMacro(Enabled);
  vtkMRMLPrintStringMacro(ViewID);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLModelPickingNode::CreateNodeInstance()
{
  return vtkMRMLModelPickingNode::New();
}
