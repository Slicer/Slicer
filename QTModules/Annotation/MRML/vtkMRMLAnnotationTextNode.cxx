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
void vtkMRMLAnnotationTextNode::Initialize(vtkMRMLScene* mrmlScene)
{
  if (!mrmlScene)
  {
    vtkErrorMacro("Scene was null!")
    return;
  }

  this->SetText(0, "This is a Text Node", 1, 1);

  mrmlScene->AddNode(this);
  this->CreateAnnotationTextDisplayNode();


  this->GetAnnotationTextDisplayNode()->SetTextScale(10);
  this->SetAnnotationAttribute(0, vtkMRMLAnnotationNode::TEXT_VISIBLE, 1);
  //this->InvokeEvent(vtkMRMLAnnotationTextNode::TextNodeAddedEvent);
  //this->InvokeEvent(vtkMRMLScene::NodeAddedEvent);
}

