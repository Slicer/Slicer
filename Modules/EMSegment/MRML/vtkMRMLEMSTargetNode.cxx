#include "vtkMRMLEMSTargetNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>

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

vtkMRMLEMSTargetNode::vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

vtkMRMLEMSTargetNode::~vtkMRMLEMSTargetNode()
{
  // nothing to do here
}

void vtkMRMLEMSTargetNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

void vtkMRMLEMSTargetNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);
}

void vtkMRMLEMSTargetNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
}

void vtkMRMLEMSTargetNode::PrintSelf(ostream& os, 
                                     vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}
