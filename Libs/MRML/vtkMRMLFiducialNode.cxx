/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMath.h"

#include "vtkMRMLFiducialNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiducialNode* vtkMRMLFiducialNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialNode");
  if(ret)
    {
    return (vtkMRMLFiducialNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiducialNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialNode");
  if(ret)
    {
    return (vtkMRMLFiducialNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiducialNode::vtkMRMLFiducialNode()
{
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->OrientationWXYZ[0] = this->OrientationWXYZ[1] = this->OrientationWXYZ[2]  = this->OrientationWXYZ[3] = 0.0;
  // so that the SetName macro won't try to free memory
  this->Name = NULL;
  this->SetName("");
  this->Selected = true;
}

//----------------------------------------------------------------------------
vtkMRMLFiducialNode::~vtkMRMLFiducialNode()
{
    if (this->Name)
    {
        delete [] this->Name;
        this->Name = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  of << " xyz=\"" << this->XYZ[0] << " " << 
                    this->XYZ[1] << " " <<
                    this->XYZ[2] << "\"";

  of << " orientationWxyz=\"" << this->OrientationWXYZ[0] << " " << 
                                this->OrientationWXYZ[1] << " " <<
                                this->OrientationWXYZ[2] << " " << 
                                this->OrientationWXYZ[3] << "\"";
  if (this->Name != NULL)
  {
      of << " name=\"" << this->Name << "\"";
  }
  of << " selected=\"" << this->Selected << "\"";
  
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "xyz")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->XYZ[0];
      ss >> this->XYZ[1];
      ss >> this->XYZ[2];
      }
    else if (!strcmp(attName, "orientationWxyz")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->OrientationWXYZ[0];
      ss >> this->OrientationWXYZ[1];
      ss >> this->OrientationWXYZ[2];
      ss >> this->OrientationWXYZ[3];
      }
    else if (!strcmp(attName, "name"))
    {
        this->SetName(attValue);
    }
    else if (!strcmp(attName, "selected"))
    {
        this->SetSelected(attValue);
    }
  }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiducialNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLFiducialNode *node = (vtkMRMLFiducialNode *) anode;

  // Vectors
  this->SetOrientationWXYZ(node->OrientationWXYZ);
  this->SetXYZ(node->XYZ);

  this->SetName(node->GetName());
  this->SetSelected(node->GetSelected());
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialNode::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "XYZ: (";
  os << indent << this->XYZ[0] << ", " << this->XYZ[1] << ", " << this->XYZ[2]
     << ") \n" ;
  
  // OrientationWXYZ
  os << indent << "OrientationWXYZ: (";
  os << indent ;
  os << this->OrientationWXYZ[0] << ", " ;
  os << this->OrientationWXYZ[1] << ", " ;
  os << this->OrientationWXYZ[2] << ", " ;
  os << this->OrientationWXYZ[3] << ")" << "\n";

  // Name:
  os << indent << "Name: " << (this->Name ? this->Name : "(none)") << "\n";

  os << indent << "Selected: " << this->Selected << "\n";
}

//-----------------------------------------------------------

void vtkMRMLFiducialNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  vtkMRMLFiducialListNode *node  = dynamic_cast < vtkMRMLFiducialListNode *>(parentNode);
  if (node)
    {
    node->AddFiducialNode(this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialNode::SetOrientationWXYZFromMatrix4x4(vtkMatrix4x4 *mat)
{
    // copied from: vtkTransform::GetOrientationWXYZ 
    int i;


    // convenient access to matrix
    double (*matrix)[4] = mat->Element;
    double ortho[3][3];
    double wxyz[4];

    for (i = 0; i < 3; i++)
    {   ortho[0][i] = matrix[0][i];
        ortho[1][i] = matrix[1][i];
        ortho[2][i] = matrix[2][i];
    }
    if (vtkMath::Determinant3x3(ortho) < 0)
    {   ortho[0][i] = -ortho[0][i];
        ortho[1][i] = -ortho[1][i];
        ortho[2][i] = -ortho[2][i];
    }

    vtkMath::Matrix3x3ToQuaternion(ortho, wxyz);

    // calc the return value wxyz
    double mag = sqrt(wxyz[1]*wxyz[1] + wxyz[2]*wxyz[2] + wxyz[3]*wxyz[3]);

    if (mag)
    {   wxyz[0] = 2.0*acos(wxyz[0])/vtkMath::DoubleDegreesToRadians();
        wxyz[1] /= mag;
        wxyz[2] /= mag;
        wxyz[3] /= mag;
    }
    else
    {   wxyz[0] = 0.0;
        wxyz[1] = 0.0;
        wxyz[2] = 0.0;
        wxyz[3] = 1.0;
    } 
    this->OrientationWXYZ[0] = (float) wxyz[0];
    this->OrientationWXYZ[1] = (float) wxyz[1];
    this->OrientationWXYZ[2] = (float) wxyz[2];
    this->OrientationWXYZ[3] = (float) wxyz[3];
}
