#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "vtkMRMLAnnotationROINode.h"
#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMath.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLAnnotationROINode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationROINode");
  if(ret)
    {
    return (vtkMRMLAnnotationROINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationROINode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationROINode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationROINode");
  if(ret)
    {
    return (vtkMRMLAnnotationROINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationROINode;
}


//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode::vtkMRMLAnnotationROINode()
{
  this->HideFromEditors = false;
  this->ROIAnnotationFormat = const_cast<char*>("%.1f");
  this->Resolution = 5;
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::Initialize(vtkMRMLScene* mrmlScene)
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

    // default position
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->RadiusXYZ[0] = this->RadiusXYZ[1] = this->RadiusXYZ[2] = 10.0;
  this->InsideOut = 0;
  this->IJK[0] = this->IJK[1] = this->IJK[2] = 0;
  this->RadiusIJK[0] = this->RadiusIJK[1] = this->RadiusIJK[2] = 0;
  // so that the SetLabelText macro won't try to free memory
  this->LabelText = NULL;
  this->Selected = 0;
  this->VolumeNodeID = NULL;
  this->Visibility = 1;
  this->InteractiveMode = 1;
  this->HideFromEditors = 0;

  this->ROIMeasurement.clear();
  this->ROIMeasurement.push_back(20.0);
  this->ROIMeasurement.push_back(20.0);
  this->ROIMeasurement.push_back(20.0);

  this->InvokeEvent(vtkMRMLAnnotationROINode::ROINodeAddedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode::~vtkMRMLAnnotationROINode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));
  if (this->ROIAnnotationFormat)
    {
      //delete [] this->ROIAnnotationFormat;
      this->ROIAnnotationFormat = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);

  of << indent << " rulerDistanceAnnotationFormat=\"";
  if (this->ROIAnnotationFormat) 
    {
      of << this->ROIAnnotationFormat << "\"";
    }
  else 
    {
      of << "\"";
    }
  of << indent << " rulerResolution=\""<< this->Resolution << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ReadXMLAttributes(const char** atts)
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
    this->SetROIAnnotationFormat(attValue.c_str());
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
 // vtkMRMLAnnotationROINode *node = (vtkMRMLAnnotationROINode *) anode;

 // this->SetResolution(node->GetResolution());
 // this->SetROIAnnotationFormat(node->GetROIAnnotationFormat());
}

//-----------------------------------------------------------
void vtkMRMLAnnotationROINode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode 
  // => will be taken care of by vtkMRMLModelDisplayNode  

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Not necessary bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode 
  // => will be taken care of  in vtkMRMLModelNode
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
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
  if (this->ROIAnnotationFormat) 
    {
      os  << this->ROIAnnotationFormat << "\n";
    }
  else 
    {
      os  << "(None)" << "\n";
    }
  os << indent << "rulerResolution: " << this->Resolution << "\n";
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationROINode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 1) {
    vtkErrorMacro("AnnotationRuler: "<< this->GetName() << " cannot have more than 3 control points !");
    return -1;
  }
  return Superclass::AddControlPoint(newControl,selectedFlag,visibleFlag);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationROINode::GetROIAnnotationScale()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetROIAnnotationScale(double init)
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
void vtkMRMLAnnotationROINode::SetROIAnnotationVisibility(int flag)
{
  this->SetAnnotationAttribute(0,vtkMRMLAnnotationNode::TEXT_VISIBLE,flag);

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationROINode::GetROIAnnotationVisibility()
{
  return this->GetAnnotationAttribute(0, vtkMRMLAnnotationNode::TEXT_VISIBLE);
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationROINode::SetControlPoint(int id, double newControl[3])
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
double* vtkMRMLAnnotationROINode::GetPointColour()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetPointColour(double initColor[3])
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
double* vtkMRMLAnnotationROINode::GetROIAnnotationTextColour()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetROIAnnotationTextColour(double initColor[3])
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
double* vtkMRMLAnnotationROINode::GetLineColour()
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetLineColour(double initColor[3])
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
void vtkMRMLAnnotationROINode::ApplyTransform(vtkMatrix4x4* transformMatrix)
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
void vtkMRMLAnnotationROINode::ApplyTransform(vtkAbstractTransform* transform)
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

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetXYZ(double X, double Y, double Z)
{
  this->XYZ[0] = X;
  this->XYZ[1] = Y;
  this->XYZ[2] = Z;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetXYZ(double* xyz)
{
  this->SetXYZ(xyz[0], xyz[1], xyz[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ)
{ 
  this->RadiusXYZ[0] = RadiusX;
  this->RadiusXYZ[1] = RadiusY;
  this->RadiusXYZ[2] = RadiusZ;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetRadiusXYZ(double* radiusXYZ)
{
  this->SetRadiusXYZ(radiusXYZ[0], radiusXYZ[1], radiusXYZ[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetBounds(double* bounds)
{
  this->SetBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetBounds(double boundsX1, double boundsX2, double boundsY1, double boundsY2, double boundsZ1, double boundsZ2)
{
  this->Bounds[0] = boundsX1;
  this->Bounds[1] = boundsX2;
  this->Bounds[2] = boundsY1;
  this->Bounds[3] = boundsY2;
  this->Bounds[4] = boundsZ1;
  this->Bounds[5] = boundsZ2;

  this->Modified();
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetROIMeasurement(double val1, double val2, double val3)
{
  this->ROIMeasurement.clear();
  this->ROIMeasurement.push_back(val1);
  this->ROIMeasurement.push_back(val2);
  this->ROIMeasurement.push_back(val3);

  this->InvokeEvent(vtkMRMLAnnotationROINode::ValueModifiedEvent);
}

//---------------------------------------------------------------------------
std::vector<double> vtkMRMLAnnotationROINode::GetROIMeasurement()
{
  return this->ROIMeasurement;
}
