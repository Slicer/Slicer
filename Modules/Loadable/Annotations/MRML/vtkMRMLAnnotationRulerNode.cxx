// MRML includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationRulerStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkObjectFactory.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationRulerNode);


//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode::vtkMRMLAnnotationRulerNode()
{
  this->HideFromEditors = false;
  this->DistanceAnnotationFormat = nullptr;
  this->SetDistanceAnnotationFormat("%.0f mm");
  this->ModelID1 = nullptr;
  this->ModelID2 = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationRulerNode::~vtkMRMLAnnotationRulerNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->DistanceAnnotationFormat)
    {
      delete [] this->DistanceAnnotationFormat;
      this->DistanceAnnotationFormat = nullptr;
    }
  if (this->ModelID1)
    {
    delete [] this->ModelID1;
    this->ModelID1 = nullptr;
    }
  if (this->ModelID2)
    {
    delete [] this->ModelID2;
    this->ModelID2 = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " rulerDistanceAnnotationFormat=\"";
  if (this->DistanceAnnotationFormat)
    {
    of << this->DistanceAnnotationFormat << "\"";
    }
  else
    {
    of << "\"";
    }

  if (this->ModelID1)
    {
    of << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << " modelID2=\"" << this->ModelID2 << "\"";
    }
  of << " distanceMeasurement=\"" << this->GetDistanceMeasurement() << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationRulerNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);


  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "rulerDistanceAnnotationFormat"))
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
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationRulerNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLAnnotationRulerNode *node = (vtkMRMLAnnotationRulerNode *) anode;

  //this->SetPosition1(node->GetPosition1());
  //this->SetPosition2(node->GetPosition2());
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
  this->SetDisplayVisibility(vis);

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
      return nullptr;
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
      return nullptr;
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
      return nullptr;
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
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLAnnotationRulerStorageNode"));
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationRulerNode::GetDistanceMeasurement()
{
  double distanceMeasurement = 0.0;

  double p1[4]={0,0,0,1};
  double p2[4]={0,0,0,1};
  this->GetPositionWorldCoordinates1(p1);
  this->GetPositionWorldCoordinates2(p2);

  distanceMeasurement = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));

  return distanceMeasurement;
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
  for(int i=0; i < 3; ++i)
    {
    position[i] = tmp[i];
    }
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
