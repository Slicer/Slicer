

#include "vtkSlicerROILogic.h"

#include <vtkMRMLROIListNode.h>

vtkCxxRevisionMacro(vtkSlicerROILogic, "$Revision$");
vtkStandardNewMacro(vtkSlicerROILogic);


//----------------------------------------------------------------------------
vtkSlicerROILogic::vtkSlicerROILogic()
{
 
}

//----------------------------------------------------------------------------
vtkSlicerROILogic::~vtkSlicerROILogic()
{

}

//----------------------------------------------------------------------------
void vtkSlicerROILogic::ProcessMRMLEvents()
{
  // TODO: implement if needed
}

//----------------------------------------------------------------------------
void vtkSlicerROILogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


//----------------------------------------------------------------------------
vtkMRMLROIListNode *vtkSlicerROILogic::AddROIList()
{
  //this->GetMRMLScene()->SaveStateForUndo();

  vtkMRMLNode *node =
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLROIListNode");
  if (node == NULL)
    {
    return NULL;
    }
  const char *name;
  name = this->GetMRMLScene()->GetTagByClassName("vtkMRMLROIListNode");
  //  node->SetName(this->GetMRMLScene()->GetUniqueNameByString(name));
  node->SetName(this->GetMRMLScene()->GetUniqueNameByString("R"));
  this->GetMRMLScene()->AddNode(node);
  return vtkMRMLROIListNode::SafeDownCast(node);
}
