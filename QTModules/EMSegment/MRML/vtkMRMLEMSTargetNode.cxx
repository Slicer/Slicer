#include "vtkMRMLEMSTargetNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <vtkObjectFactory.h>

vtkMRMLEMSTargetNode* 
vtkMRMLEMSTargetNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");
  if(ret)
    {
    return (vtkMRMLEMSTargetNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}

vtkMRMLNode* 
vtkMRMLEMSTargetNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSTargetNode");
  if(ret)
    {
    return (vtkMRMLEMSTargetNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSTargetNode;
}



