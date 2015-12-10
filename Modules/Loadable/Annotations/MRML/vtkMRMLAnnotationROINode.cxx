
// MRMLAnnotation includes
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"

// MRML includes
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkGeneralTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
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
  this->LabelText = NULL;
  this->SetLabelText("");
  this->VolumeNodeID = NULL;
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
  if (this->LabelText)
    {
    delete [] this->LabelText;
    this->LabelText = NULL;
    }
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
    << XYZ[0] << " " << XYZ[1] << " " << XYZ[2] << "\"";

  of << indent << " radiusXYZ=\""
    << RadiusXYZ[0] << " " << RadiusXYZ[1] << " " << RadiusXYZ[2] << "\"";
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
  vtkMRMLAnnotationROINode *node = vtkMRMLAnnotationROINode::SafeDownCast( anode);
  if (node)
    {
    if (node->GetLabelText())
      {
      this->SetLabelText(node->GetLabelText());
      }
    this->SetInteractiveMode(node->InteractiveMode);
    this->SetInsideOut(node->InsideOut);
    }

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
void vtkMRMLAnnotationROINode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  double (*matrix)[4] = transformMatrix->Element;

  // compute scale
  double p[] = {0,0,0,0};
  double p1[] = {1,1,1,0};
  double p2[4];
  double *v;
  double *v1;
  double v2[4];
  double scale[3];

  v = transformMatrix->MultiplyDoublePoint(p);
  v1 = transformMatrix->MultiplyDoublePoint(p1);

  int i;
  for (i=0; i<3; i++)
    {
    p2[i] = p1[i] - p[i];
    v2[i] = v1[i] - v[i];
    scale[i] = v2[i]/p2[i];
    }

  int modify = this->StartModify();

  // first point
  if (this->GetXYZ(p))
    {
    p1[0] = matrix[0][0]*p[0] + matrix[0][1]*p[1] + matrix[0][2]*p[2] + matrix[0][3];
    p1[1] = matrix[1][0]*p[0] + matrix[1][1]*p[1] + matrix[1][2]*p[2] + matrix[1][3];
    p1[2] = matrix[2][0]*p[0] + matrix[2][1]*p[1] + matrix[2][2]*p[2] + matrix[2][3];
    this->SetXYZ(p1);
    }

  // second point radius, only use scale
  if (this->GetRadiusXYZ(p))
    {
    p[0] *= scale[0];
    p[1] *= scale[1];
    p[2] *= scale[2];
    this->SetRadiusXYZ(p);
    }
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

#define AVERAGE_ABC(a,b,c) \
  c[0] = (a[0] + b[0])/2.0; \
  c[1] = (a[1] + b[1])/2.0; \
  c[2] = (a[2] + b[2])/2.0;

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
  vtkSmartPointer<vtkPoints> boxPoints = vtkSmartPointer<vtkPoints>::Take(vtkPoints::New(VTK_DOUBLE));
  boxPoints->SetNumberOfPoints(8);

  boxPoints->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  boxPoints->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  boxPoints->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  boxPoints->SetPoint(3, bounds[0], bounds[3], bounds[4]);
  boxPoints->SetPoint(4, bounds[0], bounds[2], bounds[5]);
  boxPoints->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  boxPoints->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  boxPoints->SetPoint(7, bounds[0], bounds[3], bounds[5]);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::Take(vtkPoints::New(VTK_DOUBLE));
  points->SetNumberOfPoints(6);

  double *pts =
     static_cast<vtkDoubleArray *>(boxPoints->GetData())->GetPointer(0);
  double *p0 = pts;
  double *p1 = pts + 3*1;
  double *p2 = pts + 3*2;
  double *p3 = pts + 3*3;
  //double *p4 = pts + 3*4;
  double *p5 = pts + 3*5;
  double *p6 = pts + 3*6;
  double *p7 = pts + 3*7;
  double x[3];

  AVERAGE_ABC(p0,p7,x);
  points->SetPoint(0, x);
  AVERAGE_ABC(p1,p6,x);
  points->SetPoint(1, x);
  AVERAGE_ABC(p0,p5,x);
  points->SetPoint(2, x);
  AVERAGE_ABC(p2,p7,x);
  points->SetPoint(3, x);
  AVERAGE_ABC(p1,p3,x);
  points->SetPoint(4, x);
  AVERAGE_ABC(p5,p7,x);
  points->SetPoint(5, x);

  planes->SetPoints(points);


  vtkNew<vtkDoubleArray> normals;
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(6);

  p0 = pts;
  double *px = pts + 3*1;
  double *py = pts + 3*3;
  double *pz = pts + 3*4;

  double N[6][3];
  for (i=0; i<3; i++)
    {
    N[0][i] = p0[i] - px[i];
    N[2][i] = p0[i] - py[i];
    N[4][i] = p0[i] - pz[i];
    }
  vtkMath::Normalize(N[0]);
  vtkMath::Normalize(N[2]);
  vtkMath::Normalize(N[4]);
  for (i=0; i<3; i++)
    {
    N[1][i] = -N[0][i];
    N[3][i] = -N[2][i];
    N[5][i] = -N[4][i];
    }

  double factor = (this->InsideOut ? -1.0 : 1.0);

  for (i=0; i<6; i++)
    {
    normals->SetTuple3(i, factor*N[i][0], factor*N[i][1], factor*N[i][2]);
    }
  planes->SetNormals(normals.GetPointer());

  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode != NULL)
    {
    vtkNew<vtkGeneralTransform> transform;
    tnode->GetTransformFromWorld(transform.GetPointer());
    planes->SetTransform(transform.GetPointer());
  }
  planes->Modified();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationROINode::GetRASBounds(double bounds[6])
{
  double bounds_Local[6]={0};
  vtkMath::UninitializeBounds(bounds_Local);
  if (this->GetPolyData() == NULL)
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
  bounds_Local[0]=centerPoint[0]-radius[0];
  bounds_Local[1]=centerPoint[0]+radius[0];
  bounds_Local[2]=centerPoint[1]-radius[1];
  bounds_Local[3]=centerPoint[1]+radius[1];
  bounds_Local[4]=centerPoint[2]-radius[2];
  bounds_Local[5]=centerPoint[2]+radius[2];

  this->TransformBoundsToRAS(bounds_Local, bounds);
}
