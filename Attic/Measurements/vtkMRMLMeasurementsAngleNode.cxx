#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLMeasurementsAngleNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"
#include "vtkMath.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLMeasurementsAngleNode, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


//------------------------------------------------------------------------------
vtkMRMLMeasurementsAngleNode* vtkMRMLMeasurementsAngleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsAngleNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsAngleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsAngleNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLMeasurementsAngleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsAngleNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsAngleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsAngleNode;
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementsAngleNode::vtkMRMLMeasurementsAngleNode()
{
  this->HideFromEditors = false;
  
  // default starting position
  this->SetPosition1(-100.0, 0.0, 50.0);
  this->SetPosition2(100.0,  0.0, 50.0);
  this->SetPositionCenter(0.0, 0.0, 0.0);

  // the annotation on the line
  this->LabelFormat = NULL;
  this->SetLabelFormat("%.0f");
  this->CurrentAngleAnnotation = NULL;
  this->SetLabelScale(10.0, 10.0, 10.0);
  this->LabelVisibilityOn();

  // visibility of elements of the widget
  this->Ray1VisibilityOn();
  this->Ray2VisibilityOn();
  this->ArcVisibilityOn();
  
  // the end points of the lines are blue, they're cloned so can't have a
  // different colour for each end
  this->SetPointColour(0.2, 0.5, 0.8);
  this->SetPoint2Colour(1.0, 0.8, 0.7);
  this->SetPointCentreColour(1.0, 1.0, 1.0);
  
  // line colour
  this->SetLineColour(1.0, 1.0, 1.0);

  // arc colour
  this->SetArcColour(1.0, 0.0, 0.0);

  // text colour
  this->SetLabelTextColour(1.0, 0.0, 0.0);

  // default taken from vtkLineRepresentation
  this->SetResolution(5);

  this->ModelID1 = NULL;
  this->ModelID2 = NULL;
  this->ModelIDCenter = NULL;
}



//----------------------------------------------------------------------------
vtkMRMLMeasurementsAngleNode::~vtkMRMLMeasurementsAngleNode()
{
    vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));
  if (this->LabelFormat)
    {
    delete [] this->LabelFormat;
    this->LabelFormat = NULL;
    }
  if (this->CurrentAngleAnnotation)
    {
    delete [] this->CurrentAngleAnnotation;
    this->CurrentAngleAnnotation = NULL;
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
  if (this->ModelIDCenter)
    {
    delete [] this->ModelIDCenter;
    this->ModelIDCenter = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::WriteXML(ostream& of, int nIndent)
{

  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " position1=\"" << this->Position1[0] << " " << this->Position1[1] << " " << this->Position1[2] << "\"";
  of << indent << " position2=\"" << this->Position2[0] << " " << this->Position2[1] << " " << this->Position2[2] << "\"";
  of << indent << " positionCenter=\"" << this->PositionCenter[0] << " " << this->PositionCenter[1] << " " << this->PositionCenter[2] << "\"";
  of << indent << " angle=\"" << this->Angle << "\"";

  of << indent << " labelFormat=\"" << this->LabelFormat << "\"";
  of << indent << " labelScale=\"" << this->LabelScale[0] << " " << this->LabelScale[1] << " " << this->LabelScale[2] << "\"";
  of << indent << " labelVisibility=\"" << (this->LabelVisibility  ? "true" : "false") << "\"";

  of << indent << " ray1Visibility=\"" << (this->Ray1Visibility  ? "true" : "false") << "\"";
  of << indent << " ray2Visibility=\"" << (this->Ray2Visibility  ? "true" : "false") << "\"";
  of << indent << " arcVisibility=\"" << (this->ArcVisibility  ? "true" : "false") << "\"";

  of << indent << " pointColour=\"" << this->PointColour[0] << " " << this->PointColour[1] << " " << this->PointColour[2] << "\"";
  of << indent << " point2Colour=\"" << this->Point2Colour[0] << " " << this->Point2Colour[1] << " " << this->Point2Colour[2] << "\"";
  of << indent << " pointCentreColour=\"" << this->PointCentreColour[0] << " " << this->PointCentreColour[1] << " " << this->PointCentreColour[2] << "\"";

  of << indent << " lineColour=\"" << this->LineColour[0] << " " << this->LineColour[1] << " " << this->LineColour[2] << "\"";
  of << indent << " arcColour=\"" << this->ArcColour[0] << " " << this->ArcColour[1] << " " << this->ArcColour[2] << "\"";
  of << indent << " textColour=\"" << this->LabelTextColour[0] << " " << this->LabelTextColour[1] << " " << this->LabelTextColour[2] << "\"";

  of << indent << " resolution=\"" << this->Resolution << "\"";
  if (this->ModelID1)
    {
    of << indent << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << indent << " modelID2=\"" << this->ModelID2 << "\"";
    }
  if (this->ModelIDCenter)
    {
    of << indent << " modelIDCenter=\"" << this->ModelIDCenter << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "position1"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->Position1[i] = val;
        }
      }
    else if (!strcmp(attName, "position2"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->Position2[i] = val;
        }
      }
    else if (!strcmp(attName, "positionCenter"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->PositionCenter[i] = val;
        }
      }
    else if (!strcmp(attName, "angle"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      ss >> val;
      this->Angle = val;
      }
    else if (!strcmp(attName, "labelFormat"))
      {
      this->SetLabelFormat(attValue);
      }
    else if (!strcmp(attName, "labelScale"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->LabelScale[i] = val;
        }
      }
    else if (!strcmp(attName, "labelVisibility"))
      {
      if (!strcmp(attValue,"true")) 
        {
        this->LabelVisibility = 1;
        }
      else
        {
        this->LabelVisibility = 0;
        }
      }
    else if (!strcmp(attName, "ray1Visibility"))
      {
      if (!strcmp(attValue,"true")) 
        {
        this->Ray1Visibility = 1;
        }
      else
        {
        this->Ray1Visibility = 0;
        }
      }
    else if (!strcmp(attName, "ray2Visibility"))
      {
      if (!strcmp(attValue,"true")) 
        {
        this->Ray2Visibility = 1;
        }
      else
        {
        this->Ray2Visibility = 0;
        }
      }
    else if (!strcmp(attName, "arcVisibility"))
      {
      if (!strcmp(attValue,"true")) 
        {
        this->ArcVisibility = 1;
        }
      else
        {
        this->ArcVisibility = 0;
        }
      }
    else if (!strcmp(attName, "pointColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->PointColour[i] = val;
        }
      }
    else if (!strcmp(attName, "point2Colour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->Point2Colour[i] = val;
        }
      }
    else if (!strcmp(attName, "pointCentreColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->PointCentreColour[i] = val;
        }
      }
    else if (!strcmp(attName, "lineColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->LineColour[i] = val;
        }
      }
    else if (!strcmp(attName, "arcColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->ArcColour[i] = val;
        }
      }

    else if (!strcmp(attName, "textColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->LabelTextColour[i] = val;
        }
      }
    else if (!strcmp(attName, "resolution"))
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      this->Resolution = val;
      }
    else if (!strcmp(attName, "modelID1"))
      {
      this->SetModelID1(attValue);
      }
    else if (!strcmp(attName, "modelID2"))
      {
      this->SetModelID2(attValue);
      }
    else if (!strcmp(attName, "modelIDCenter"))
      {
      this->SetModelIDCenter(attValue);
      }
    }
  this->UpdateCurrentAngleAnnotation();
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMeasurementsAngleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLMeasurementsAngleNode *node = (vtkMRMLMeasurementsAngleNode *) anode;

  this->SetPosition1 ( node->GetPosition1() );
  this->SetPosition2 ( node->GetPosition2() );
  this->SetPositionCenter ( node->GetPositionCenter() );
  this->SetAngle  ( node->GetAngle()  );

  this->SetLabelFormat(node->GetLabelFormat());
  this->SetCurrentAngleAnnotation(node->GetCurrentAngleAnnotation());
  this->SetLabelScale(node->GetLabelScale());
  this->SetLabelVisibility(node->GetLabelVisibility());

  this->SetRay1Visibility(node->GetRay1Visibility());
  this->SetRay2Visibility(node->GetRay2Visibility());
  this->SetArcVisibility(node->GetArcVisibility());
  
  this->SetPointColour(node->GetPointColour());
  this->SetPoint2Colour(node->GetPoint2Colour());
  this->SetPointCentreColour(node->GetPointCentreColour());
  this->SetLineColour(node->GetLineColour());
  this->SetArcColour(node->GetArcColour());
  this->SetLabelTextColour(node->GetLabelTextColour());

  this->SetResolution(node->GetResolution());

  this->SetModelID1(node->GetModelID1());
  this->SetModelID2(node->GetModelID2());
  this->SetModelIDCenter(node->GetModelIDCenter());
}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Position1: ";
  double *pos = this->GetPosition1();
  if (pos)
    {
    os << pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
    }
  else
    {
    os << "NULL";
    }
  os << indent << "Position2: ";
  pos = this->GetPosition2();
  if (pos)
    {
    os << pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
    }
  else
    {
    os << "NULL";
    }
  os << indent << "Position Center: ";
  pos = this->GetPositionCenter();
  if (pos)
    {
    os << pos[0] << ", " << pos[1] << ", " << pos[2] << "\n";
    }
  else
    {
    os << "NULL";
    }
  os << indent << "Angle: " << this->Angle << "\n";
  os << indent << "Label Format: " << (this->LabelFormat != NULL ? this->LabelFormat : "NULL") << "\n";
  os << indent << "Current Angle Annotation: " << (this->CurrentAngleAnnotation != NULL ? this->CurrentAngleAnnotation : "NULL") << "\n";
  os << indent << "Label Scale: " << this->LabelScale[0] << " " << this->LabelScale[1] << " " << this->LabelScale[2] << "\n";
  os << indent << "Label Visibility: " << this->LabelVisibility << "\n";

  os << indent << "Ray 1 Visibility: " << this->Ray1Visibility << "\n";
  os << indent << "Ray 2 Visibility: " << this->Ray2Visibility << "\n";
  os << indent << "Arc Visibility:" << this->ArcVisibility << "\n";
  
  os << indent << "Point Colour: " << this->PointColour[0] << " " << this->PointColour[1] << " " << this->PointColour[2] << "\n";
  os << indent << "Point 2 Colour: " << this->Point2Colour[0] << " " << this->Point2Colour[1] << " " << this->Point2Colour[2] << "\n";
  os << indent << "Point Centre Colour: " << this->PointCentreColour[0] << " " << this->PointCentreColour[1] << " " << this->PointCentreColour[2] << "\n";
  os << indent << "Line Colour: " << this->LineColour[0] << " " << this->LineColour[1] << " " << this->LineColour[2] << "\n";
  os << indent << "Arc Colour: " << this->ArcColour[0] << " " << this->ArcColour[1] << " " << this->ArcColour[2] << "\n";
  os << indent << "Text Colour: " << this->LabelTextColour[0] << " " << this->LabelTextColour[1] << " " << this->LabelTextColour[2] << "\n";

  
  
  os << indent << "Resolution: " << this->Resolution << "\n";

  os << indent << "Model 1: " << (this->ModelID1 ? this->ModelID1 : "none") << "\n";
  os << indent << "Model 2: " << (this->ModelID2 ? this->ModelID2 : "none") << "\n";
  os << indent << "Model Center: " << (this->ModelIDCenter ? this->ModelIDCenter : "none") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::ApplyTransform(vtkMatrix4x4* transformMatrix)
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
  // center
  p = this->GetPositionCenter();
  if (p)
    {
    xyzIn[0] = p[0];
    xyzIn[1] = p[1];
    xyzIn[2] = p[2];

    xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
    xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
    xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
    this->SetPositionCenter(xyzOut);
    }
  
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::ApplyTransform(vtkAbstractTransform* transform)
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

//---------------------------------------------------------------------------
double vtkMRMLMeasurementsAngleNode::GetAngle()
{
  // taken from vtkAngleRepresentation3D.cxx BuildRepresentation
  double p1[3], p2[3], c[3], vector2[3], vector1[3];
  double l1 = 0.0, l2 = 0.0;
  this->GetPosition1(p1);
  this->GetPositionCenter(c);
  this->GetPosition2(p2);

  vector1[0] = p1[0] - c[0];
  vector1[1] = p1[1] - c[1];
  vector1[2] = p1[2] - c[2];
  vector2[0] = p2[0] - c[0];
  vector2[1] = p2[1] - c[1];
  vector2[2] = p2[2] - c[2];
  l1 = vtkMath::Normalize( vector1 );
  l2 = vtkMath::Normalize( vector2 );
  double val = acos( vtkMath::Dot( vector1, vector2 ) );

  // and convert to degrees
  double degrees = vtkMath::DegreesFromRadians(val);
  
  if (fabs(degrees - this->Angle) > 0.001)
    {
    this->SetAngle(degrees);
    this->UpdateCurrentAngleAnnotation();
    }
  return this->Angle;
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementsAngleNode::UpdateCurrentAngleAnnotation()
{
  if (this->GetLabelFormat())
    {
    char str[1024];
    sprintf(str, this->LabelFormat, this->GetAngle());
    this->SetCurrentAngleAnnotation(str);
    }
  else
    {
    this->SetCurrentAngleAnnotation(NULL);
    }
}
