#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "vtkMRMLAnnotationBidimensionalNode.h"
#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMath.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationBidimensionalNode* vtkMRMLAnnotationBidimensionalNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationBidimensionalNode");
  if(ret)
    {
    return (vtkMRMLAnnotationBidimensionalNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationBidimensionalNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationBidimensionalNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationBidimensionalNode");
  if(ret)
    {
    return (vtkMRMLAnnotationBidimensionalNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationBidimensionalNode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationBidimensionalNode::vtkMRMLAnnotationBidimensionalNode()
{
  this->HideFromEditors = false;
  this->AnnotationFormat = NULL;
  this->SetAnnotationFormat("%.0f mm");
  this->Resolution = 5;
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::Initialize(vtkMRMLScene* mrmlScene)
{
    if (!mrmlScene)
    {
        vtkErrorMacro("Scene was null!");
        return;
    }

    mrmlScene->AddNode(this);

    this->CreateAnnotationTextDisplayNode();
    this->CreateAnnotationPointDisplayNode();
    this->CreateAnnotationLineDisplayNode();
    
    this->AddText(" ",1,1);

    this->InvokeEvent(vtkMRMLAnnotationBidimensionalNode::BidimensionalNodeAddedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationBidimensionalNode::~vtkMRMLAnnotationBidimensionalNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));
  if (this->AnnotationFormat)
    {
      delete [] this->AnnotationFormat;
      this->AnnotationFormat = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  of << indent << "AnnotationFormat=\"";
  if (this->AnnotationFormat) 
    {
      of << this->AnnotationFormat << "\"";
    }
  else 
    {
      of << "\"";
    }
  of << indent << "Resolution=\""<< this->Resolution << "\"";


}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationRulerNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);

  
  while (*atts != NULL) 
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "rulerResolution"))       
      {

    std::stringstream ss;
        ss << attValue;
        ss >> this->Resolution;
      }
    else if (!strcmp(attName, "AnnotationFormat"))
      {
    this->SetAnnotationFormat(attValue.c_str());
      }

    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//-----------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  //cout << "vtkMRMLAnnotationRulerNode::PrintAnnotationInfo" << endl;
  if (titleFlag) 
    {
      
      os <<indent << "vtkMRMLAnnotationBidimensionalNode: Annotation Summary";
      if (this->GetName()) 
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  os << indent << "rulerDistanceAnnotationFormat: ";
  if (this->AnnotationFormat) 
    {
      os  << this->AnnotationFormat << "\n";
    }
  else 
    {
      os  << "(None)" << "\n";
    }
  os << indent << "Resolution: " << this->Resolution << "\n";
}

