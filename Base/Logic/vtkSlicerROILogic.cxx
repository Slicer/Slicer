
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkSlicerROILogic.h"
 
#include "vtkMRMLROINode.h"
#include "vtkMRMLSelectionNode.h"

vtkCxxRevisionMacro(vtkSlicerROILogic, "$Revision: 1.0 $");
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
  name = this->MRMLScene->GetTagByClassName("vtkMRMLROIListNode");
  //  node->SetName(this->MRMLScene->GetUniqueNameByString(name));
  node->SetName(this->MRMLScene->GetUniqueNameByString("R"));
  this->GetMRMLScene()->AddNode(node);
  return vtkMRMLROIListNode::SafeDownCast(node);
}
