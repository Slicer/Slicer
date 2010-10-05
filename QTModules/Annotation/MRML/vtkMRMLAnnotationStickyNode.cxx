#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationStickyNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

//------------------------------------------------------------------------------
vtkMRMLAnnotationStickyNode* vtkMRMLAnnotationStickyNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationStickyNode");
  if(ret)
    {
    return (vtkMRMLAnnotationStickyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationStickyNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationStickyNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationStickyNode");
  if(ret)
    {
    return (vtkMRMLAnnotationStickyNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationStickyNode;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationStickyNode::SetSticky(const char* text,int selectedFlag)
{
    if (text) 
    {
        this->SetStickyText(text);
    }
    else 
    {
        this->SetStickyText("");
    }

    this->SetSelected(selectedFlag);

    return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationStickyNode::Initialize(vtkMRMLScene* mrmlScene)
{
    if (!mrmlScene)
    {
        vtkErrorMacro("Scene was null!");
        return;
    }

    mrmlScene->AddNode(this);

    this->CreateAnnotationTextDisplayNode();
    this->AddText("",1,1);

}



