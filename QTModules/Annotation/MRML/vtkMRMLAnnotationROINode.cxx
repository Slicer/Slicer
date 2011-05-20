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
  this->LabelText = NULL;
  this->SetLabelText("");
  this->VolumeNodeID = NULL;
  this->HideFromEditors = false;
  
  // default placement of the widget as in Slicer3
  this->SetXYZ(0,0,0);
  this->SetRadiusXYZ(10,10,10);
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::Initialize(vtkMRMLScene* mrmlScene)
{
  Superclass::Initialize(mrmlScene);

}
//    if (!mrmlScene)
//    {
//        vtkErrorMacro("Scene was null!");
//        return;
//    }
//
//    mrmlScene->AddNode(this);
//
//    this->CreateAnnotationTextDisplayNode();
//    this->CreateAnnotationPointDisplayNode();
//    this->CreateAnnotationLineDisplayNode();
//
//    this->AddText(" ",1,1);
//
//
//    // so that the SetLabelText macro won't try to free memory
//    this->Selected = 0;
//    this->VolumeNodeID = NULL;
//    this->InteractiveMode = 1;
//    this->HideFromEditors = 0;
//
//    this->InvokeEvent(vtkMRMLAnnotationROINode::ROINodeAddedEvent);
//}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode::~vtkMRMLAnnotationROINode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));

}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->VolumeNodeID != NULL)
    {
    of << indent << " volumeNodeID=\"" << this->VolumeNodeID << "\"";
    }
  if (this->LabelText != NULL)
    {
    of << indent << " labelText=\"" << this->LabelText << "\"";
    }

  // we do not have to write out the coordinates since the controlPointNode does that
  /*of << indent << " xyz=\""
    << this->XYZ[0] << " " << this->XYZ[1] << " " << this->XYZ[2] << "\"";

  of << indent << " radiusXYZ=\""
    << this->RadiusXYZ[0] << " " << this->RadiusXYZ[1] << " " << this->RadiusXYZ[2] << "\"";
  */

  of << indent << " insideOut=\"" << (this->InsideOut ? "true" : "false") << "\"";

  //of << indent << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  of << indent << " interactiveMode=\"" << (this->InteractiveMode ? "true" : "false") << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    // this is for backwards compatibility
    // we need to parse XYZ and RadiusXYZ and store the values in the new annotation controlPoints framework
    if (!strcmp(attName, "XYZ") || !strcmp(attName, "xyz"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;

      double newVal[3] = {0,0,0};

      for(int i=0; i<3; i++)
        {
        ss >> val;
        newVal[i] = val;
        }

      this->SetXYZ(newVal[0],newVal[1],newVal[2]);
      }
    if (!strcmp(attName, "RadiusXYZ") || !strcmp(attName, "radiusXYZ"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;

      double newVal[3] = {0,0,0};

      for(int i=0; i<3; i++)
        {
        ss >> val;
        newVal[i] = val;
        }

      this->SetRadiusXYZ(newVal[0],newVal[1],newVal[2]);
      }
    // end of backwards compatibility


    if (!strcmp(attName, "Selected") || !strcmp(attName, "selected"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Selected = 1;
        }
      else
        {
        this->Selected = 0;
        }
      }
    else if (!strcmp(attName, "VolumeNodeID") || !strcmp(attName, "volumeNodeID"))
      {
      this->SetVolumeNodeID(attValue);
      }
    else if (!strcmp(attName, "LabelText") || !strcmp(attName, "labelText"))
      {
      this->SetLabelText(attValue);
      }
    /*else if (!strcmp(attName, "Visibility") || !strcmp(attName, "visibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility = 1;
        }
      else
        {
        this->Visibility = 0;
        }
      }*/
    else if (!strcmp(attName, "InteractiveMode") || !strcmp(attName, "interactiveMode"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->InteractiveMode = 1;
        }
      else
        {
        this->InteractiveMode = 0;
        }
      }
    else if (!strcmp(attName, "InsideOut") || !strcmp(attName, "insideOut"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->InsideOut = 1;
        }
      else
        {
        this->InsideOut = 0;
        }
      }

    }

  this->EndModify(disabledModify);
  
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::Copy(vtkMRMLNode *anode)
{

  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  //  vtkObject::Copy(anode);
  vtkMRMLAnnotationROINode *node = (vtkMRMLAnnotationROINode *) anode;
  this->SetLabelText(node->GetLabelText());
  this->SetInteractiveMode(node->InteractiveMode);
  this->SetInsideOut(node->InsideOut);

  this->EndModify(disabledModify);

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
  //cout << "vtkMRMLAnnotationROINode::PrintAnnotationInfo" << endl;
  if (titleFlag) 
    {
      
      os <<indent << "vtkMRMLAnnotationROINode: Annotation Summary";
      if (this->GetName()) 
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  double origin[3];
  this->GetXYZ(origin);
  os << indent << "Origin:" << endl;
  os << indent << indent << "X: " << origin[0] << endl;
  os << indent << indent << "Y: " << origin[1] << endl;
  os << indent << indent << "Z: " << origin[2] << endl;

  double radius[3];
  this->GetRadiusXYZ(radius);
  os << indent << "Radius:" << endl;
  os << indent << indent << "rX: " << radius[0] << endl;
  os << indent << indent << "rY: " << radius[1] << endl;
  os << indent << indent << "rZ: " << radius[2] << endl;

}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationROINode::AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag)
{
  if (this->GetNumberOfControlPoints() > 1) {
    vtkErrorMacro("AnnotationROINode: "<< this->GetName() << " cannot have more than 2 control points !");
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
      vtkErrorMacro("AnnotationROI: "<< this->GetName() << " cannot get AnnotationTextDisplayNode");
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
bool vtkMRMLAnnotationROINode::GetXYZ(double point[3])
{
  point[0] = point[1] = point[2] = 0.0;
  if (this->PolyData && this->PolyData->GetPoints()) 
    {
    this->PolyData->GetPoint(0, point);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationROINode::GetRadiusXYZ(double point[3])
{
  point[0] = point[1] = point[2] = 0.0;
  if (this->PolyData && this->PolyData->GetPoints()) 
    {
    this->PolyData->GetPoint(1, point);
    return true;
    }
  return false;
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
double* vtkMRMLAnnotationROINode::GetPointColor()
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetPointColor(double initColor[3])
{
  vtkMRMLAnnotationPointDisplayNode *node = this->GetAnnotationPointDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationROI: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationROINode::GetROIAnnotationTextColor()
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetROIAnnotationTextColor(double initColor[3])
{
  vtkMRMLAnnotationTextDisplayNode *node = this->GetAnnotationTextDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationROI: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
      return;
    }
  node->SetSelectedColor(initColor);
}

//---------------------------------------------------------------------------
double* vtkMRMLAnnotationROINode::GetLineColor()
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      return 0;
    }
  return node->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::SetLineColor(double initColor[3])
{
  vtkMRMLAnnotationLineDisplayNode *node = this->GetAnnotationLineDisplayNode();
  if (!node)
    {
      vtkErrorMacro("AnnotationROI: "<< this->GetName() << " cannot get AnnotationPointDisplayNode");
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
  double p[3];

  // first point
  if (this->GetXYZ(p))
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
  
    xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
    xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
    xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
    this->SetXYZ(xyzOut);
    }

  // second point
  if (this->GetRadiusXYZ(p))
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
    xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
    xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
    this->SetRadiusXYZ(xyzOut);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ApplyTransform(vtkAbstractTransform* transform)
{
  double xyzIn[3];
  double xyzOut[3];
  double p[3];

  // first point
  if (this->GetXYZ(p))
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
    
    transform->TransformPoint(xyzIn,xyzOut);
    this->SetXYZ(xyzOut);
    }
  
  // second point
  if (this->GetRadiusXYZ(p))
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];
    
    transform->TransformPoint(xyzIn,xyzOut);
    this->SetRadiusXYZ(xyzOut);
    }
}

