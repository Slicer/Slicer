#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLMeasurementsRulerNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkAbstractTransform.h"

#include "vtkMath.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLMeasurementsRulerNode, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


//------------------------------------------------------------------------------
vtkMRMLMeasurementsRulerNode* vtkMRMLMeasurementsRulerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsRulerNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsRulerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsRulerNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLMeasurementsRulerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLMeasurementsRulerNode");
  if(ret)
    {
      return (vtkMRMLMeasurementsRulerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMeasurementsRulerNode;
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementsRulerNode::vtkMRMLMeasurementsRulerNode()
{
  this->HideFromEditors = false;
  
  // default starting position
  this->SetPosition1(-100.0, 50.0, 0.0);
  this->SetPosition2(100.0,  50.0, 0.0);
  
  // the annotation on the line
  this->DistanceAnnotationFormat = NULL;
  this->SetDistanceAnnotationFormat("%.1f mm");
  this->CurrentDistanceAnnotation = NULL;
  this->UpdateCurrentDistanceAnnotation();
  this->SetDistanceAnnotationScale(10.0, 10.0, 10.0);
  this->DistanceAnnotationVisibilityOn();

  // the end points of the line
  this->SetPointColour(0.9, 0.5, 0.5);
  this->SetPoint2Colour(0.9, 0.7, 0.9);
  // line colour
  this->SetLineColour(1.0, 1.0, 1.0);

  // text colour
  this->SetDistanceAnnotationTextColour(1.0, 0.0, 0.0);

  // default taken from vtkLineRepresentation
  this->SetResolution(5);

  this->ModelID1 = NULL;
  this->ModelID2 = NULL;

  this->Distance = 0.0;
}



//----------------------------------------------------------------------------
vtkMRMLMeasurementsRulerNode::~vtkMRMLMeasurementsRulerNode()
{
    vtkDebugMacro("Destructing...." << (this->GetID() != NULL ? this->GetID() : "null id"));
  if (this->DistanceAnnotationFormat)
    {
    delete [] this->DistanceAnnotationFormat;
    this->DistanceAnnotationFormat = NULL;
    }
  if (this->CurrentDistanceAnnotation)
    {
    delete [] this->CurrentDistanceAnnotation;
    this->CurrentDistanceAnnotation = NULL;
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
void vtkMRMLMeasurementsRulerNode::WriteXML(ostream& of, int nIndent)
{

  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " position1=\"" << this->Position1[0] << " " << this->Position1[1] << " " << this->Position1[2] << "\"";
  of << indent << " position2=\"" << this->Position2[0] << " " << this->Position2[1] << " " << this->Position2[2] << "\"";
  of << indent << " distance=\"" << this->Distance << "\"";

  of << indent << " distanceAnnotationFormat=\"" << this->DistanceAnnotationFormat << "\"";
  of << indent << " distanceAnnotationScale=\"" << this->DistanceAnnotationScale[0] << " " << this->DistanceAnnotationScale[1] << " " << this->DistanceAnnotationScale[2] << "\"";
  of << indent << " distanceAnnotationVisibility=\"" << (this->DistanceAnnotationVisibility  ? "true" : "false") << "\"";

  of << indent << " pointColour=\"" << this->PointColour[0] << " " << this->PointColour[1] << " " << this->PointColour[2] << "\"";
  of << indent << " point2Colour=\"" << this->Point2Colour[0] << " " << this->Point2Colour[1] << " " << this->Point2Colour[2] << "\"";
  of << indent << " lineColour=\"" << this->LineColour[0] << " " << this->LineColour[1] << " " << this->LineColour[2] << "\"";
  of << indent << " textColour=\"" << this->DistanceAnnotationTextColour[0] << " " << this->DistanceAnnotationTextColour[1] << " " << this->DistanceAnnotationTextColour[2] << "\"";

  of << indent << " resolution=\"" << this->Resolution << "\"";
  if (this->ModelID1)
    {
    of << indent << " modelID1=\"" << this->ModelID1 << "\"";
    }
  if (this->ModelID2)
    {
    of << indent << " modelID2=\"" << this->ModelID2 << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsRulerNode::ReadXMLAttributes(const char** atts)
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
    else if (!strcmp(attName, "distance"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      ss >> val;
      this->Distance = val;
      }
    else if (!strcmp(attName, "distanceAnnotationFormat"))
      {
      this->SetDistanceAnnotationFormat(attValue);
      }
    else if (!strcmp(attName, "distanceAnnotationScale"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->DistanceAnnotationScale[i] = val;
        }
      }
    else if (!strcmp(attName, "distanceAnnotationVisibility"))
      {
      if (!strcmp(attValue,"true")) 
        {
        this->DistanceAnnotationVisibility = 1;
        }
      else
        {
        this->DistanceAnnotationVisibility = 0;
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
    else if (!strcmp(attName, "textColour"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++) 
        {
        ss >> val;
        this->DistanceAnnotationTextColour[i] = val;
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
    }
  this->UpdateCurrentDistanceAnnotation();
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLMeasurementsRulerNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLMeasurementsRulerNode *node = (vtkMRMLMeasurementsRulerNode *) anode;

  this->SetPosition1 ( node->GetPosition1() );
  this->SetPosition2 ( node->GetPosition2() );
  this->SetDistance  ( node->GetDistance()  );
  this->SetDistanceAnnotationFormat(node->GetDistanceAnnotationFormat());
  this->SetDistanceAnnotationScale(node->GetDistanceAnnotationScale());
  this->SetDistanceAnnotationVisibility(node->GetDistanceAnnotationVisibility());
  this->SetCurrentDistanceAnnotation(node->GetCurrentDistanceAnnotation());
  this->SetPointColour(node->GetPointColour());
  this->SetPoint2Colour(node->GetPoint2Colour());
  this->SetLineColour(node->GetLineColour());
  this->SetDistanceAnnotationTextColour(node->GetDistanceAnnotationTextColour());

  this->SetResolution(node->GetResolution());

  this->SetModelID1(node->GetModelID1());
  this->SetModelID2(node->GetModelID2());
}


//----------------------------------------------------------------------------
void vtkMRMLMeasurementsRulerNode::PrintSelf(ostream& os, vtkIndent indent)
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
  os << indent << "Distance: " << this->Distance << "\n";
  os << indent << "Distance Annotation Format: " << this->DistanceAnnotationFormat << "\n";
  os << indent << "Distance Annotation Scale: " << this->DistanceAnnotationScale[0] << " " << this->DistanceAnnotationScale[1] << " " << this->DistanceAnnotationScale[2] << "\n";
  os << indent << "Distance Annotation Visibility: " << this->DistanceAnnotationVisibility << "\n";
  os << indent << "Current Distance Annotation: " << (this->CurrentDistanceAnnotation != NULL ? this->CurrentDistanceAnnotation : "NULL") << "\n";
  
  os << indent << "Point Colour: " << this->PointColour[0] << " " << this->PointColour[1] << " " << this->PointColour[2] << "\n";
  os << indent << "Point 2 Colour: " << this->Point2Colour[0] << " " << this->Point2Colour[1] << " " << this->Point2Colour[2] << "\n";
  os << indent << "Line Colour: " << this->LineColour[0] << " " << this->LineColour[1] << " " << this->LineColour[2] << "\n";
  os << indent << "Text Colour: " << this->DistanceAnnotationTextColour[0] << " " << this->DistanceAnnotationTextColour[1] << " " << this->DistanceAnnotationTextColour[2] << "\n";

  os << indent << "Resolution: " << this->Resolution << "\n";

  os << indent << "Model 1: " << (this->ModelID1 ? this->ModelID1 : "none") << "\n";
  os << indent << "Model 2: " << (this->ModelID2 ? this->ModelID2 : "none") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurementsRulerNode::ApplyTransform(vtkMatrix4x4* transformMatrix)
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
void vtkMRMLMeasurementsRulerNode::ApplyTransform(vtkAbstractTransform* transform)
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
double vtkMRMLMeasurementsRulerNode::GetDistance()
{
  double val = sqrt(vtkMath::Distance2BetweenPoints(this->Position1, this->Position2));
  if (fabs(val - this->Distance) > 0.001)
    {
    this->SetDistance(val);
    this->UpdateCurrentDistanceAnnotation();
    }
  return this->Distance;
}
//---------------------------------------------------------------------------
void vtkMRMLMeasurementsRulerNode::UpdateCurrentDistanceAnnotation()
{
  if (this->GetDistanceAnnotationFormat())
    {
    char str[1024];
    sprintf(str, this->DistanceAnnotationFormat, this->GetDistance());
    this->SetCurrentDistanceAnnotation(str);
    }
  else
    {
    this->SetCurrentDistanceAnnotation(NULL);
    }
}
