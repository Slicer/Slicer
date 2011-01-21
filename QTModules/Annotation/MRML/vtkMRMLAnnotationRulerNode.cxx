#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMath.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode* vtkMRMLAnnotationRulerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationRulerNode");
  if(ret)
    {
    return (vtkMRMLAnnotationRulerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationRulerNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationRulerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationRulerNode");
  if(ret)
    {
    return (vtkMRMLAnnotationRulerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationRulerNode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode::vtkMRMLAnnotationRulerNode()
{
  this->HideFromEditors = false;
  this->DistanceAnnotationFormat = NULL;
  this->SetDistanceAnnotationFormat("%.0f mm");
  this->Resolution = 5;
  this->ModelID1 = NULL;
  this->ModelID2 = NULL;
}
//----------------------------------------------------------------------------
// void vtkMRMLAnnotationRulerNode::Initialize(vtkMRMLScene* mrmlScene)
// {
//     if (!mrmlScene)
//     {
//         vtkErrorMacro("Scene was null!");
//         return;
//     }
// 
//   mrmlScene->AddNode(this);
// 
//     this->CreateAnnotationTextDisplayNode();
//     this->CreateAnnotationPointDisplayNode();
//     this->CreateAnnotationLineDisplayNode();
//     
//     this->AddText(" ",1,1);
// 
//   // default starting position
//   this->SetPosition1(-250.0, 50.0, 0.0);
//   this->SetPosition2(250.0, 50.0, 0.0);
// 
//   // the end points of the lines are blue
//     double color1[3] = { 0.0, 0.0, 1.0}; 
//     this->SetPointColour(color1);
// 
//     // line color
//     double color2[3] = { 1.0, 1.0, 1.0}; 
//     this->SetLineColour(color2);
// 
//     // text color
//     double color3[3] = { 1.0, 0.0, 0.0 } ;
//     this->SetDistanceAnnotationTextColour(color3);
// 
//     // the annotation on the line
//     //this->SetDistanceAnnotationScale(10.0);
//     //this->SetDistanceAnnotationVisibility(1);
// 
//     // default taken from vtkLineRepresentation
//     this->SetResolution(5);
// 
//     this->distanceMeasurement = 500.0;
// 
//     this->InvokeEvent(vtkMRMLAnnotationRulerNode::RulerNodeAddedEvent);
// }

//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode::~vtkMRMLAnnotationRulerNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));
  if (this->DistanceAnnotationFormat)
    {
      delete [] this->DistanceAnnotationFormat;
      this->DistanceAnnotationFormat = NULL;
    }
  if (this->ModelID1)
    {
    delete [] this->ModelID1;
    this->ModelID1 = NULL;
    }
  if (this->ModelID2)
    {
    delete [] this->ModelID2;
    this->ModelID2 = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  of << indent << " rulerDistanceAnnotationFormat=\"";
  if (this->DistanceAnnotationFormat) 
    {
      of << this->DistanceAnnotationFormat << "\"";
    }
  else 
    {
      of << "\"";
    }
  of << indent << " rulerResolution=\""<< this->Resolution << "\"";

  if (this->ModelID1)
    {
    of << indent << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << indent << " modelID2=\"" << this->ModelID2 << "\"";
    }
  if (this->distanceMeasurement)
    {
    of << indent << " distanceMeasurement=\"" << this->distanceMeasurement << "\"";
    }

}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::ReadXMLAttributes(const char** atts)
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
    else if (!strcmp(attName, "rulerDistanceAnnotationFormat"))
      {
      this->SetDistanceAnnotationFormat(attValue.c_str());
      }

    else if (!strcmp(attName, "modelID1"))
      {
      this->SetModelID1(attValue.c_str());
      }
    else if (!strcmp(attName, "modelID2"))
      {
      this->SetModelID2(attValue.c_str());
      }
    else if (!strcmp(attName, "distanceMeasurement"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->distanceMeasurement;
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  vtkMRMLAnnotationRulerNode *node = (vtkMRMLAnnotationRulerNode *) anode;

  //this->SetPosition1(node->GetPosition1());
  //this->SetPosition2(node->GetPosition2());
  this->SetDistanceMeasurement(node->GetDistanceMeasurement());
  //this->SetResolution(node->GetResolution());
  //this->SetDistanceAnnotationFormat(node->GetDistanceAnnotationFormat());
  //this->SetModelID1(node->GetModelID1());
  //this->SetModelID2(node->GetModelID2());
}

//-----------------------------------------------------------
void vtkMRMLAnnotationRulerNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode 
  // => will be taken care of by vtkMRMLModelDisplayNode  

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Not necessary bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode 
  // => will be taken care of  in vtkMRMLModelNode
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  //cout << "vtkMRMLAnnotationRulerNode::PrintAnnotationInfo" << endl;
  if (titleFlag) 
    {
      
      os <<indent << "vtkMRMLAnnotationRulerNode: Annotation Summary";
      if (this->GetName()) 
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  os << indent << "rulerDistanceAnnotationFormat: ";
  if (this->DistanceAnnotationFormat) 
    {
      os  << this->DistanceAnnotationFormat << "\n";
    }
  else 
    {
      os  << "(None)" << "\n";
    }
  os << indent << "rulerResolution: " << this->Resolution << "\n";

  os << indent << "Model 1: " << (this->ModelID1 ? this->ModelID1 : "none") << "\n";
  os << indent << "Model 2: " << (this->ModelID2 ? this->ModelID2 : "none") << "\n";

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationRulerNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 1) {
    vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot have more than 3 control points !");
    return -1;
  }
  return Superclass::AddControlPoint(newControl,selectedFlag,visibleFlag);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationRulerNode::GetDistanceAnnotationScale()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetDistanceAnnotationScale(double init)
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationTextDisplayNode");
      return;
    }
  node->SetTextScale(init);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetDistanceAnnotationVisibility(int flag)
{
  this->SetAnnotationAttribute(0,vtkMRMLAnnotationNode::TEXT_VISIBLE,flag);

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationRulerNode::GetDistanceAnnotationVisibility()
{
  return this->GetAnnotationAttribute(0, vtkMRMLAnnotationNode::TEXT_VISIBLE);
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationRulerNode::SetRuler(vtkIdType line1Id, int sel, int vis)
{
  vtkIdType line1IDPoints[2];
  this->GetEndPointsId(line1Id,line1IDPoints);

  //Change this later
  if (line1IDPoints[0]!= 0 || line1IDPoints[1] != 1)
    {
      vtkErrorMacro("Not valid line definition!");
      return -1;
    }
  this->SetSelected(sel); 
  this->SetVisible(vis); 

  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationRulerNode::SetControlPoint(int id, double newControl[3])
{
  if (id < 0 || id > 1) {
    return 0;
  }

  int flag = Superclass::SetControlPoint(id, newControl,1,1);
  if (!flag) 
    {
      return 0;
    }
  if (this->GetNumberOfControlPoints() < 2) 
    {
      return 1;
    }

  this->AddLine(0,1,1,1);
  return 1;
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationRulerNode::GetPointColour()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetPointColour(double initColor[3])
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationRulerNode::GetDistanceAnnotationTextColour()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetDistanceAnnotationTextColour(double initColor[3])
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationRulerNode::GetLineColour()
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetLineColour(double initColor[3])
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::ApplyTransform(vtkMatrix4x4* transformMatrix)
{
  double (*matrix)[4] = transformMatrix->Element;
  double xyzIn[3];
  double xyzOut[3];
  double *p = NULL;

  // first point
  p = this->GetPosition1();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
  
    xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
    xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
    xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
    this->SetPosition1(xyzOut);
    }

  // second point
  p = this->GetPosition2();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
    xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
    xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
    this->SetPosition2(xyzOut);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::ApplyTransform(vtkAbstractTransform* transform)
{
  double xyzIn[3];
  double xyzOut[3];
  double *p;

  // first point
  p = this->GetPosition1();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
    
    transform->TransformPoint(xyzIn,xyzOut);
    this->SetPosition1(xyzOut);
    }
  
  // second point
  p = this->GetPosition2();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
    
    transform->TransformPoint(xyzIn,xyzOut);
    this->SetPosition2(xyzOut);
    }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLAnnotationRulerNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLAnnotationRulerStorageNode::New());
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::SetDistanceMeasurement(double val)
{
    this->distanceMeasurement = val;
    this->InvokeEvent(vtkMRMLAnnotationRulerNode::ValueModifiedEvent);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationRulerNode::GetDistanceMeasurement()
{
    return this->distanceMeasurement;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::GetPosition1(double position[3])
{
  double * tmp = this->GetControlPointCoordinates(0);
  if (!tmp)
    {
    vtkErrorMacro("AnnotationRuler - Failed to get first control point");
    return;
    }
  for(int i=0; i < 3; ++i){ position[i] = tmp[i]; }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::GetPosition2(double position[3])
{
  double * tmp = this->GetControlPointCoordinates(1);
  if (!tmp)
    {
    vtkErrorMacro("AnnotationRuler - Failed to get second control point");
    return;
    }
  for(int i=0; i < 3; ++i){ position[i] = tmp[i]; }
}
