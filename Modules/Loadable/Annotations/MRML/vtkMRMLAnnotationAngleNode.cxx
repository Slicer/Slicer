//MRML includes
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationAngleNode);


//----------------------------------------------------------------------------
vtkMRMLAnnotationAngleNode::vtkMRMLAnnotationAngleNode()
{
  this->HideFromEditors = false;
  this->LabelFormat = nullptr;
  this->SetLabelFormat("%-#6.3g");
  this->Resolution = 5;
  this->ModelID1 = nullptr;
  this->ModelID2 = nullptr;
  this->ModelIDCenter = nullptr;
}
//----------------------------------------------------------------------------
// void vtkMRMLAnnotationAngleNode::Initialize(vtkMRMLScene* mrmlScene)
// {
//   if (!mrmlScene)
//     {
//       vtkErrorMacro("Scene was null!");
//       return;
//     }
//
//   mrmlScene->AddNode(this);
//   this->CreateAnnotationTextDisplayNode();
//   this->CreateAnnotationPointDisplayNode();
//   this->CreateAnnotationLineDisplayNode();
//
//   this->AddText(" ",1,1);
//
//   // default starting position
//   {
//     double pos[3] = {-100.0, 0.0, 50.0};
//     this->SetPosition1(pos);
//   }
//   {
//     double pos[3] = {100.0, 0.0, 50.0};
//     this->SetPosition2(pos);
//   }
//   {
//     double pos[3] = {0.0, 0.0, 0.0};
//     this->SetPositionCenter(pos);
//   }
//
//   // the annotation on the line
//   this->SetLabelScale(10.0);
//   this->SetLabelVisibility(1);
//
//   // visibility of elements of the widget
//   this->SetRay1Visibility(1);
//   this->SetRay2Visibility(1);
//   this->SetArcVisibility(1);
//
//   // the end points of the lines are blue, they're cloned so can't have a
//   // different colour for each end
//   {
//     double color[3] = { 0.0, 0.0, 1.0};
//     this->SetPointColour(color);
//   }
//   // line colour
//   {
//     double color[3] = { 1.0, 1.0, 1.0};
//     this->SetLineColour(color);
//   }
//   // text colour
//   {
//     double color[3] = { 1.0, 0.0, 0.0 } ;
//     this->SetLabelTextColour(color);
//   }
//
//   // default taken from vtkLineRepresentation
//   this->SetResolution(5);
//
//   this->InvokeEvent(vtkMRMLAnnotationAngleNode::AngleNodeAddedEvent);
//
// }

//----------------------------------------------------------------------------
vtkMRMLAnnotationAngleNode::~vtkMRMLAnnotationAngleNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->LabelFormat)
    {
      delete [] this->LabelFormat;
      this->LabelFormat = nullptr;
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
  if (this->ModelIDCenter)
    {
    delete [] this->ModelIDCenter;
    this->ModelIDCenter = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " labelFormat=\"";
  if (this->LabelFormat)
    {
    of << this->LabelFormat << "\"";
    }
  else
    {
    of << "\"";
    }
  of << " angleResolution=\""<< this->Resolution << "\"";

  if (this->ModelID1)
    {
    of << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << " modelID2=\"" << this->ModelID2 << "\"";
    }
  if (this->ModelIDCenter)
    {
    of << " modelIDCenter=\"" << this->ModelIDCenter << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationAngleNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);

  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "angleResolution"))
      {

    std::stringstream ss;
        ss << attValue;
        ss >> this->Resolution;
      }
    else if (!strcmp(attName, "labelFormat"))
      {
    this->SetLabelFormat(attValue.c_str());
      }
    else if (!strcmp(attName, "modelID1"))
      {
    this->SetModelID1(attValue.c_str());
      }
    else if (!strcmp(attName, "modelID2"))
      {
      this->SetModelID2(attValue.c_str());
      }
    else if (!strcmp(attName, "modelIDCenter"))
      {
      this->SetModelIDCenter(attValue.c_str());
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLAnnotationAngleNode *node = (vtkMRMLAnnotationAngleNode *) anode;

  //this->SetPosition1 ( node->GetPosition1() );
  //this->SetPosition2 ( node->GetPosition2() );
  //this->SetPositionCenter ( node->GetPositionCenter() );

  // this->SetLabelFormat(node->GetLabelFormat());
  //this->SetLabelScale(node->GetLabelScale());
  //this->SetLabelVisibility(node->GetLabelVisibility());

  //this->SetRay1Visibility(node->GetRay1Visibility());
  //this->SetRay2Visibility(node->GetRay2Visibility());
  //this->SetArcVisibility(node->GetArcVisibility());

  //this->SetPointColour(node->GetPointColour());
  //this->SetLineColour(node->GetLineColour());
  //this->SetLabelTextColour(node->GetLabelTextColour());

  /*
  this->SetResolution(node->GetResolution());
  this->SetLabelFormat(node->GetLabelFormat());

  this->SetModelID1(node->GetModelID1());
  this->SetModelID2(node->GetModelID2());
  this->SetModelIDCenter(node->GetModelIDCenter());
  */
}

//-----------------------------------------------------------
void vtkMRMLAnnotationAngleNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of by vtkMRMLModelDisplayNode

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Not necessary bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of  in vtkMRMLModelNode
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  //cout << "vtkMRMLAnnotationAngleNode::PrintAnnotationInfo" << endl;
  if (titleFlag)
    {

      os <<indent << "vtkMRMLAnnotationAngleNode: Annotation Summary";
      if (this->GetName())
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  os << indent << "angleLabelFormat: ";
  if (this->LabelFormat)
    {
      os  << this->LabelFormat << "\n";
    }
  else
    {
      os  << "(None)" << "\n";
    }
  os << indent << "angleResolution: " << this->Resolution << "\n";

  os << indent << "Model 1: " << (this->ModelID1 ? this->ModelID1 : "none") << "\n";
  os << indent << "Model 2: " << (this->ModelID2 ? this->ModelID2 : "none") << "\n";
  os << indent << "Model Center: " << (this->ModelIDCenter ? this->ModelIDCenter : "none") << "\n";

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationAngleNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 2) {
    vtkErrorMacro("AnnotationAngle: "<< this->GetName() << " cannot have more than 3 control points !");
    return -1;
  }
  return Superclass::AddControlPoint(newControl,selectedFlag,visibleFlag);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationAngleNode::GetLabelScale()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetLabelScale(double init)
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();

  if (!node)
    {
      vtkErrorMacro("AnnotationAngle: "<< this->GetName() << " cannot get AnnotationDisplayNode");
      return;
    }
  node->SetTextScale(init);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetLabelVisibility(int flag)
{
  this->SetAnnotationAttribute(0,vtkMRMLAnnotationNode::TEXT_VISIBLE,flag);

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationAngleNode::GetLabelVisibility()
{
  return this->GetAnnotationAttribute(0, vtkMRMLAnnotationNode::TEXT_VISIBLE);
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationAngleNode::SetAngle(vtkIdType line1Id, vtkIdType line2Id, int sel, int vis)
{
  vtkIdType line1IDPoints[2];
  this->GetEndPointsId(line1Id,line1IDPoints);
  vtkIdType line2IDPoints[2];
  this->GetEndPointsId(line2Id,line2IDPoints);

  //Change this later to the code below
  if (line1IDPoints[0]!= 0 || line1IDPoints[1] != 1 || line2IDPoints[0] != 1 || line2IDPoints[1] != 2)
    {
      vtkErrorMacro("Not valid line definition!");
      return -1;
    }
  //this->SetPosition1ByID();
  //this->SetPosition2ByID();
  //this->SetPositionCenterByID();

  this->SetSelected(sel);
  this->SetDisplayVisibility(vis);

  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationAngleNode::SetControlPoint(int id, double newControl[3])
{
  if (id < 0 || id > 2) {
    return 0;
  }

  int flag = Superclass::SetControlPoint(id, newControl,1,1);
  if (!flag)
    {
      return 0;
    }
  if (this->GetNumberOfControlPoints() < 3)
    {
      return 1;
    }


  if (this->GetNumberOfLines() == 2)
    {
      return 1;
    }

  this->AddLine(0,1,1,1);
  this->AddLine(1,2,1,1);
  return 1;
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationAngleNode::GetPointColour()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetPointColour(double initColor[3])
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationAngle: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationAngleNode::GetLabelTextColour()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetLabelTextColour(double initColor[3])
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationAngle: "<< this->GetName() << " cannot get AnnotationTextDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationAngleNode::GetLineColour()
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetLineColour(double initColor[3])
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationAngle: "<< this->GetName() << " cannot get AnnotationLineDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::ApplyTransform(vtkAbstractTransform* transform)
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

  // center point
  p = this->GetPositionCenter();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    transform->TransformPoint(xyzIn,xyzOut);
    this->SetPositionCenter(xyzOut);
    }
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLAnnotationAngleNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLAnnotationAngleStorageNode"));
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationAngleNode::SetAngleMeasurement(double val)
{
    this->angleMeasurement = val;
    this->InvokeEvent(vtkMRMLAnnotationAngleNode::ValueModifiedEvent);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationAngleNode::GetAngleMeasurement()
{
    return this->angleMeasurement;
}
