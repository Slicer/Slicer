#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode* vtkMRMLAnnotationHierarchyNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationHierarchyNode");
  if(ret)
    {
    return (vtkMRMLAnnotationHierarchyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationHierarchyNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode::vtkMRMLAnnotationHierarchyNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode::~vtkMRMLAnnotationHierarchyNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::ReadXMLAttributes( const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of,indent);
}
