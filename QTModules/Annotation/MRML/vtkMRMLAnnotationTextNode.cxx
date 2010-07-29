#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationTextNode.h"
#include "vtkBitArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"


//------------------------------------------------------------------------------
vtkMRMLAnnotationTextNode* vtkMRMLAnnotationTextNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationTextNode");
  if(ret)
    {
    return (vtkMRMLAnnotationTextNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationTextNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationTextNode");
  if(ret)
    {
    return (vtkMRMLAnnotationTextNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextNode;
}

//----------------------------------------------------------------------------
// void vtkMRMLAnnotationTextNode::Initialize(vtkMRMLScene* mrmlScene)
// {
//   if (!mrmlScene)
//   {
//     vtkErrorMacro("Scene was null!")
//     return;
//   }
// 
//   Superclass::Initialize(mrmlScene);
//   this->SetTextLabel("New text");
//   this->GetAnnotationTextDisplayNode()->SetTextScale(10); 
// }

