#include <sstream>

#include "vtkObjectFactory.h"
#include <vtkGeneralTransform.h>
#include "vtkPlanes.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"

#include "vtkMath.h"

#include "vtkMRMLROINode.h"
#include "vtkMRMLTransformNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLROINode);

//----------------------------------------------------------------------------
void vtkMRMLROINode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  os << "XYZ:\n";
  os << indent << XYZ[0] << " " << XYZ[1] << " " << XYZ[2] << "\n";
  os << "Radius XYZ:\n";
  os << indent << RadiusXYZ[0] << " " << RadiusXYZ[1] << " " << RadiusXYZ[2] << "\n";
  return;
}

//----------------------------------------------------------------------------
vtkMRMLROINode::vtkMRMLROINode()
{
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->RadiusXYZ[0] = this->RadiusXYZ[1] = this->RadiusXYZ[2] = 10.0;
  this->InsideOut = 0;

  this->IJK[0] = this->IJK[1] = this->IJK[2] = 0;
  this->RadiusIJK[0] = this->RadiusIJK[1] = this->RadiusIJK[2] = 0;
  // so that the SetLabelText macro won't try to free memory
  this->LabelText = nullptr;
  this->SetLabelText("");
  this->Selected = 0;
  this->VolumeNodeID = nullptr;
  this->Visibility = 1;
  this->InteractiveMode = 1;
  this->HideFromEditors = 0;
  return;
}

//----------------------------------------------------------------------------
vtkMRMLROINode::~vtkMRMLROINode()
{
  if (this->LabelText)
    {
    delete [] this->LabelText;
    this->LabelText = nullptr;
    }
  if (this->ID)
    {
    delete [] this->ID;
    this->ID = nullptr;
    }
  if (this->VolumeNodeID)
    {
    delete [] this->VolumeNodeID;
    this->VolumeNodeID = nullptr;
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROINode::WriteXML(ostream& of, int nIndent)
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

  of << " xyz=\""
    << this->XYZ[0] << " " << this->XYZ[1] << " " << this->XYZ[2] << "\"";

  of << " radiusXYZ=\""
    << this->RadiusXYZ[0] << " " << this->RadiusXYZ[1] << " " << this->RadiusXYZ[2] << "\"";

  of << " insideOut=\"" << (this->InsideOut ? "true" : "false") << "\"";

  of << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  of << " interactiveMode=\"" << (this->InteractiveMode ? "true" : "false") << "\"";

  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROINode::ReadXMLAttributes( const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "XYZ") || !strcmp(attName, "xyz"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        ss >> val;
        this->XYZ[i] = val;
        }
      }
    if (!strcmp(attName, "RadiusXYZ") || !strcmp(attName, "radiusXYZ"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        ss >> val;
        this->RadiusXYZ[i] = val;
        }
      }
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
    else if (!strcmp(attName, "Visibility") || !strcmp(attName, "visibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility = 1;
        }
      else
        {
        this->Visibility = 0;
        }
      }
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

  return;
}

void vtkMRMLROINode::ReadXMLString(const char *keyValuePairs)
{
  // used because the ROI list gloms together the point's key and
  // values into one long string, VERY dependent on the order it's written
  // out in when WriteXML is used

  // insert the string into a stream
  std::stringstream ss;
  ss << keyValuePairs;

  char keyName[1024];

  // get out the id
  ss >> keyName;
  ss >> this->ID;
  vtkDebugMacro("ReadXMLString: got id " << this->ID);

  // get out the volume id
  ss >> keyName;
  if (!strcmp(keyName, "VolumeNodeID"))
    {
    char* IDValue = new char[1024];
    ss >> IDValue;
    this->SetVolumeNodeID(IDValue);
    delete [] IDValue;
    vtkDebugMacro("ReadXMLString: got VolumeNodeID " << this->VolumeNodeID);
    }
  else
    {
    // now get the label text value
    ss >> this->LabelText;
    vtkDebugMacro("ReadXMLString: got label text " << this->LabelText);
    }

  // get the xyz key
  ss >> keyName;
  // now get the x, y, z values
  ss >> this->XYZ[0];
  ss >> this->XYZ[1];
  ss >> this->XYZ[2];

  // get the Radiusxyz key
  ss >> keyName;
  // now get the x, y, z values
  ss >> this->RadiusXYZ[0];
  ss >> this->RadiusXYZ[1];
  ss >> this->RadiusXYZ[2];

  // get the selected flag
  ss >> keyName;
  ss >> this->Selected;
  return;
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLROINode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  //  vtkObject::Copy(anode);
  vtkMRMLROINode *node = (vtkMRMLROINode *) anode;
  this->SetXYZ(node->XYZ);
  this->SetRadiusXYZ(node->RadiusXYZ);
  this->SetLabelText(node->GetLabelText());
  this->SetSelected(node->GetSelected());
  this->SetVisibility(node->Visibility);
  this->SetInteractiveMode(node->InteractiveMode);
  this->SetInsideOut(node->InsideOut);

  this->EndModify(disabledModify);

  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::UpdateReferences()
{
  Superclass::UpdateReferences();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::ProcessMRMLEvents ( vtkObject *caller,
                                        unsigned long event,
                                        void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetXYZ(double X, double Y, double Z)
{
  this->XYZ[0] = X;
  this->XYZ[1] = Y;
  this->XYZ[2] = Z;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetXYZ(double* xyz)
{
  this->SetXYZ(xyz[0], xyz[1], xyz[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ)
{
  this->RadiusXYZ[0] = RadiusX;
  this->RadiusXYZ[1] = RadiusY;
  this->RadiusXYZ[2] = RadiusZ;

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetRadiusXYZ(double* radiusXYZ)
{
  this->SetRadiusXYZ(radiusXYZ[0], radiusXYZ[1], radiusXYZ[2]);
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetIJK(double I, double J, double K)
{
  this->IJK[0] = I;
  this->IJK[1] = J;
  this->IJK[2] = K;

  //Update

  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetIJK(double* ijk)
{
  this->SetIJK(ijk[0], ijk[1], ijk[2]);
  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetRadiusIJK(double RadiusI, double RadiusJ, double RadiusK)
{
  this->RadiusIJK[0] = RadiusI;
  this->RadiusIJK[1] = RadiusJ;
  this->RadiusIJK[2] = RadiusK;

  //Update

  this->Modified();
  return;
}

//-----------------------------------------------------------------------------
void vtkMRMLROINode::SetRadiusIJK(double* radiusIJK)
{
  this->SetRadiusIJK(radiusIJK[0], radiusIJK[1], radiusIJK[2]);
  return;
}

//---------------------------------------------------------------------------
bool vtkMRMLROINode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLROINode::ApplyTransform(vtkAbstractTransform* transform)
{
  double *xyzIn  = this->GetXYZ();
  double xyzOut[3];
  transform->TransformPoint(xyzIn,xyzOut);
  this->SetXYZ(xyzOut);
}


#define VTK_AVERAGE(a,b,c) \
  c[0] = (a[0] + b[0])/2.0; \
  c[1] = (a[1] + b[1])/2.0; \
  c[2] = (a[2] + b[2])/2.0;

void vtkMRMLROINode::GetTransformedPlanes(vtkPlanes *planes)
{
  double bounds[6];
  int i;

  for (i=0; i<3; i++)
    {
    bounds[2*i  ] = this->XYZ[i] - this->RadiusXYZ[i];
    bounds[2*i+1] = this->XYZ[i] + this->RadiusXYZ[i];
    }
  vtkPoints *boxPoints = vtkPoints::New(VTK_DOUBLE);
  boxPoints->SetNumberOfPoints(8);

  boxPoints->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  boxPoints->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  boxPoints->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  boxPoints->SetPoint(3, bounds[0], bounds[3], bounds[4]);
  boxPoints->SetPoint(4, bounds[0], bounds[2], bounds[5]);
  boxPoints->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  boxPoints->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  boxPoints->SetPoint(7, bounds[0], bounds[3], bounds[5]);

  vtkPoints *points = vtkPoints::New(VTK_DOUBLE);
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

  VTK_AVERAGE(p0,p7,x);
  points->SetPoint(0, x);
  VTK_AVERAGE(p1,p6,x);
  points->SetPoint(1, x);
  VTK_AVERAGE(p0,p5,x);
  points->SetPoint(2, x);
  VTK_AVERAGE(p2,p7,x);
  points->SetPoint(3, x);
  VTK_AVERAGE(p1,p3,x);
  points->SetPoint(4, x);
  VTK_AVERAGE(p5,p7,x);
  points->SetPoint(5, x);

  planes->SetPoints(points);


  vtkDoubleArray *normals = vtkDoubleArray::New();
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
  planes->SetNormals(normals);


  normals->Delete();
  boxPoints->Delete();
  points->Delete();

  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  if (tnode != nullptr) // && tnode->IsTransformToWorldLinear())
    {
    //vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    //transformToWorld->Identity();
    //tnode->GetMatrixTransformToWorld(transformToWorld);

    vtkGeneralTransform *transform = vtkGeneralTransform::New();
    tnode->GetTransformFromWorld(transform);

    //transform->Inverse();
    planes->SetTransform(transform);
  }
  planes->Modified();

}
