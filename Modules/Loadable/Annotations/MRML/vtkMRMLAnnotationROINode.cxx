
// MRMLAnnotation includes
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"

// MRML includes
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkVectorOperators.h>
#include <vtkPlanes.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationROINode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode::vtkMRMLAnnotationROINode()
{
  this->LabelText = nullptr;
  this->SetLabelText("");
  this->VolumeNodeID = nullptr;
  this->HideFromEditors = false;
  this->InsideOut = 0;
  this->InteractiveMode = 0;

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
//    this->VolumeNodeID = nullptr;
//    this->InteractiveMode = 1;
//    this->HideFromEditors = 0;
//
//    this->InvokeEvent(vtkMRMLAnnotationROINode::ROINodeAddedEvent);
//}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode::~vtkMRMLAnnotationROINode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->LabelText)
    {
    delete [] this->LabelText;
    this->LabelText = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  Superclass::WriteXML(of, nIndent);

  if (this->VolumeNodeID != nullptr)
    {
    of << " volumeNodeID=\"" << this->VolumeNodeID << "\"";
    }
  if (this->LabelText != nullptr)
    {
    of << " labelText=\"" << this->LabelText << "\"";
    }

  // we do not have to write out the coordinates since the controlPointNode does that
  /*of << " xyz=\""
    << XYZ[0] << " " << XYZ[1] << " " << XYZ[2] << "\"";

  of << " radiusXYZ=\""
    << RadiusXYZ[0] << " " << RadiusXYZ[1] << " " << RadiusXYZ[2] << "\"";
  */

  of << " insideOut=\"" << (this->InsideOut ? "true" : "false") << "\"";

  //of << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  of << " interactiveMode=\"" << (this->InteractiveMode ? "true" : "false") << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  while (*atts != nullptr)
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
void vtkMRMLAnnotationROINode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  //  vtkObject::Copy(anode);
  vtkMRMLAnnotationROINode *node = vtkMRMLAnnotationROINode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  if (node->GetLabelText())
    {
    this->SetLabelText(node->GetLabelText());
    }
  this->SetInteractiveMode(node->InteractiveMode);
  this->SetInsideOut(node->InsideOut);
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
  if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    this->Modified();
    }

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
  if (this->GetPoints())
    {
    this->GetPoints()->GetPoint(0, point);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationROINode::GetRadiusXYZ(double point[3])
{
  point[0] = point[1] = point[2] = 0.0;
  if (this->GetPoints())
    {
    this->GetPoints()->GetPoint(1, point);
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
      return nullptr;
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
      return nullptr;
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
      return nullptr;
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
void vtkMRMLAnnotationROINode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  // Unfortunately, ROI axis cannot be rotated, therefore we determine the new
  // ROI bounds so that includes all the corner points of the transformed ROI.

  double center[4] = { 0, 0, 0, 1 }; // 1 at the end: homogeneous coordinate for MultiplyDoublePoint
  double radius[3] = { 0 };
  this->GetXYZ(center);
  this->GetRadiusXYZ(radius);

  const int numberOfCornerPoints = 8;
  double cornerPoints_Local[numberOfCornerPoints][4] =
    {
    { center[0] - radius[0], center[1] - radius[1], center[2] - radius[2], 1 },
    { center[0] + radius[0], center[1] - radius[1], center[2] - radius[2], 1 },
    { center[0] - radius[0], center[1] + radius[1], center[2] - radius[2], 1 },
    { center[0] + radius[0], center[1] + radius[1], center[2] - radius[2], 1 },
    { center[0] - radius[0], center[1] - radius[1], center[2] + radius[2], 1 },
    { center[0] + radius[0], center[1] - radius[1], center[2] + radius[2], 1 },
    { center[0] - radius[0], center[1] + radius[1], center[2] + radius[2], 1 },
    { center[0] + radius[0], center[1] + radius[1], center[2] + radius[2], 1 }
    };

  vtkBoundingBox boundingBox_Transformed;
  for (int i = 0; i < numberOfCornerPoints; i++)
    {
    double* cornerPoint_Transformed = transformMatrix->MultiplyDoublePoint(cornerPoints_Local[i]);
    boundingBox_Transformed.AddPoint(cornerPoint_Transformed);
    }

  double center_Transformed[3] = { 0 };
  boundingBox_Transformed.GetCenter(center_Transformed);

  double diameters_Transformed[3] = { 0 };
  boundingBox_Transformed.GetLengths(diameters_Transformed);

  int modify = this->StartModify();
  this->SetXYZ(center_Transformed);
  this->SetRadiusXYZ(diameters_Transformed[0] / 2, diameters_Transformed[1] / 2, diameters_Transformed[2] / 2);
  this->EndModify(modify);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::ApplyTransform(vtkAbstractTransform* transform)
{
  vtkHomogeneousTransform* linearTransform = vtkHomogeneousTransform::SafeDownCast(transform);
  if (linearTransform)
    {
    this->ApplyTransformMatrix(linearTransform->GetMatrix());
    return;
    }

  vtkErrorMacro("vtkMRMLAnnotationROINode::ApplyTransform is only supported for linear transforms");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::GetTransformedPlanes(vtkPlanes *planes)
{
  double bounds[6];
  int i;

  double XYZ[3];
  double RadiusXYZ[3];

  this->GetXYZ(XYZ);
  this->GetRadiusXYZ(RadiusXYZ);

  for (i=0; i<3; i++)
    {
    bounds[2*i  ] = XYZ[i] - RadiusXYZ[i];
    bounds[2*i+1] = XYZ[i] + RadiusXYZ[i];
    }
  vtkSmartPointer<vtkPoints> boxPoints =
      vtkSmartPointer<vtkPoints>::Take(vtkPoints::New(VTK_DOUBLE));
  boxPoints->SetNumberOfPoints(8);

  boxPoints->SetPoint(0, bounds[0], bounds[2], bounds[4]); // origin
  boxPoints->SetPoint(1, bounds[1], bounds[2], bounds[4]); // x axis
  boxPoints->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  boxPoints->SetPoint(3, bounds[0], bounds[3], bounds[4]); // y axis
  boxPoints->SetPoint(4, bounds[0], bounds[2], bounds[5]); // z axis
  boxPoints->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  boxPoints->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  boxPoints->SetPoint(7, bounds[0], bounds[3], bounds[5]);


  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode != nullptr)
    {
    vtkNew<vtkGeneralTransform> transform;
    tnode->GetTransformToWorld(transform.GetPointer());

    for(unsigned int idx = 0; idx < 8; ++idx)
      {
      double oldPoint[3] = {0., 0., 0.};
      boxPoints->GetPoint(idx, oldPoint);

      double newPoint[3] = {0., 0., 0.};
      transform->TransformPoint(oldPoint, newPoint);
      boxPoints->SetPoint(idx, newPoint);
      }
  }

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::Take(vtkPoints::New(VTK_DOUBLE));
  points->SetNumberOfPoints(6);

  double *pts =
     static_cast<vtkDoubleArray *>(boxPoints->GetData())->GetPointer(0);

  // these 3 planes contain pts[0]
  points->SetPoint(0, pts);
  points->SetPoint(1, pts);
  points->SetPoint(2, pts);
  // these 3 planes contain pts[6]
  points->SetPoint(3, pts + 3 * 6);
  points->SetPoint(4, pts + 3 * 6);
  points->SetPoint(5, pts + 3 * 6);

  planes->SetPoints(points);

  double factor = (this->InsideOut ? -1.0 : 1.0);

  // compute normals
  vtkNew<vtkDoubleArray> normals;
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(6);

  vtkVector3d origin;
  vtkVector3d pointU;
  vtkVector3d pointV;
  vtkVector3d normal;

  double * p0 = pts;
  origin.Set(p0[0], p0[1], p0[2]);

  // x plane
  double * p1 = pts + 3 * 4; // z offset
  double * p2 = pts + 3 * 3; // y offset
  pointU.Set(p1[0], p1[1], p1[2]);
  pointV.Set(p2[0], p2[1], p2[2]);
  pointU = pointU - origin;
  pointV = pointV - origin;
  normal = pointU.Cross(pointV);
  normal.Normalize();
  normals->SetTuple3(
        0, factor*normal[0], factor*normal[1], factor*normal[2]);
  normal = normal * -1;
  normals->SetTuple3(
        3, factor*normal[0], factor*normal[1], factor*normal[2]);

  // y plane
  p1 = pts + 3 * 1; // x offset
  p2 = pts + 3 * 4; // z offset
  pointU.Set(p1[0], p1[1], p1[2]);
  pointV.Set(p2[0], p2[1], p2[2]);
  pointU = pointU - origin;
  pointV = pointV - origin;
  normal = pointU.Cross(pointV);
  normal.Normalize();
  normals->SetTuple3(
        1, factor*normal[0], factor*normal[1], factor*normal[2]);
  normal = normal * -1;
  normals->SetTuple3(
        4, factor*normal[0], factor*normal[1], factor*normal[2]);

  // z plane
  p1 = pts + 3 * 3; // y offset
  p2 = pts + 3 * 1; // x offset
  pointU.Set(p1[0], p1[1], p1[2]);
  pointV.Set(p2[0], p2[1], p2[2]);
  pointU = pointU - origin;
  pointV = pointV - origin;
  normal = pointU.Cross(pointV);
  normal.Normalize();
  normals->SetTuple3(
        2, factor*normal[0], factor*normal[1], factor*normal[2]);
  normal = normal * -1;
  normals->SetTuple3(
        5, factor*normal[0], factor*normal[1], factor*normal[2]);

  planes->SetNormals(normals.GetPointer());

  planes->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::GetBounds(double bounds[6])
{
  vtkMath::UninitializeBounds(bounds);
  if (this->GetPolyData() == nullptr)
    {
    return;
    }
  double centerPoint[3]={0};
  if (!this->GetXYZ(centerPoint))
    {
    return;
    }
  double radius[3]={0};
  if (!this->GetRadiusXYZ(radius))
    {
    return;
    }
  bounds[0]=centerPoint[0]-radius[0];
  bounds[1]=centerPoint[0]+radius[0];
  bounds[2]=centerPoint[1]-radius[1];
  bounds[3]=centerPoint[1]+radius[1];
  bounds[4]=centerPoint[2]-radius[2];
  bounds[5]=centerPoint[2]+radius[2];
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::GetRASBounds(double bounds[6])
{
  this->GetBounds(bounds);
  this->TransformBoundsToRAS(bounds, bounds);
}
