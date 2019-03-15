#include <sstream>

#include "vtkMRMLAnnotationSplineNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLScene.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationSplineNode);


//----------------------------------------------------------------------------
vtkMRMLAnnotationSplineNode::vtkMRMLAnnotationSplineNode()
{
  this->HideFromEditors = false;
  this->DistanceAnnotationFormat = const_cast<char*>("%.2f mm");
  this->Resolution = 5;
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::Initialize(vtkMRMLScene* mrmlScene)
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

  // default starting position
  {
    double pos[3] = {0.0, 0.0, 0.0};
    this->SetControlPoint(pos, 0);
  }
  {
    double pos[3] = {0.0, 0.0, 0.0};
    this->SetControlPoint(pos, 1);
  }
  {
    double pos[3] = {0.0, 0.0, 0.0};
    this->SetControlPoint(pos, 2);
  }
  {
    double pos[3] = {0.0, 0.0, 0.0};
    this->SetControlPoint(pos, 3);
  }
  {
    double pos[3] = {0.0, 0.0, 0.0};
    this->SetControlPoint(pos, 4);
  }


    this->AddText(" ",1,1);

  this->splineMeasurement = 0.0;

    this->InvokeEvent(vtkMRMLAnnotationSplineNode::SplineNodeAddedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationSplineNode::~vtkMRMLAnnotationSplineNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->DistanceAnnotationFormat)
    {
      //delete [] this->DistanceAnnotationFormat;
      this->DistanceAnnotationFormat = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::WriteXML(ostream& of, int nIndent)
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
  of << " rulerResolution=\""<< this->Resolution << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationRulerNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);


  while (*atts != nullptr)
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

    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//-----------------------------------------------------------
void vtkMRMLAnnotationSplineNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  // Nothing to do at this point  bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of by vtkMRMLModelDisplayNode

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Not necessary bc vtkMRMLAnnotationDisplayNode is subclass of vtkMRMLModelDisplayNode
  // => will be taken care of  in vtkMRMLModelNode
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
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
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationSplineNode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 1) {
    vtkErrorMacro("AnnotationSpline: "<< this->GetName() << " cannot have more than 1 control points !");
    return -1;
  }
  return Superclass::AddControlPoint(newControl,selectedFlag,visibleFlag);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationSplineNode::GetDistanceAnnotationScale()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::SetDistanceAnnotationScale(double init)
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
void vtkMRMLAnnotationSplineNode::SetDistanceAnnotationVisibility(int flag)
{
  this->SetAnnotationAttribute(0,vtkMRMLAnnotationNode::TEXT_VISIBLE,flag);

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationSplineNode::GetDistanceAnnotationVisibility()
{
  return this->GetAnnotationAttribute(0, vtkMRMLAnnotationNode::TEXT_VISIBLE);
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationSplineNode::SetRuler(vtkIdType line1Id, int sel, int vis)
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
int vtkMRMLAnnotationSplineNode::SetControlPoint(double newControl[3], int id)
{
  if (id < 0 || id > 4) {
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
double* vtkMRMLAnnotationSplineNode::GetPointColour()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::SetPointColour(double initColor[3])
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
double* vtkMRMLAnnotationSplineNode::GetDistanceAnnotationTextColour()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::SetDistanceAnnotationTextColour(double initColor[3])
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
double* vtkMRMLAnnotationSplineNode::GetLineColour()
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return nullptr;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationSplineNode::SetLineColour(double initColor[3])
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
void vtkMRMLAnnotationSplineNode::SetSplineMeasurement(double val)
{
  this->splineMeasurement = val;
  this->InvokeEvent(vtkMRMLAnnotationSplineNode::ValueModifiedEvent);
}

//---------------------------------------------------------------------------
double vtkMRMLAnnotationSplineNode::GetSplineMeasurement()
{
  return this->splineMeasurement;
}

