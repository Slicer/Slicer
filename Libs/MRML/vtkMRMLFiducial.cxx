/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducial.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"

#include "vtkMRMLFiducial.h"
//#include "vtkMRMLFiducialListNode.h"
//#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiducial* vtkMRMLFiducial::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducial");
  if(ret)
    {
    return (vtkMRMLFiducial*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducial;
}


//----------------------------------------------------------------------------
vtkMRMLFiducial::vtkMRMLFiducial()
{
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->OrientationWXYZ[0] = this->OrientationWXYZ[1] = this->OrientationWXYZ[2]  = 0.0;
  this->OrientationWXYZ[3] = 1.0;
  // so that the SetLabelText macro won't try to free memory
  this->LabelText = NULL;
  this->SetLabelText("");
  this->Selected = true;
}

//----------------------------------------------------------------------------
vtkMRMLFiducial::~vtkMRMLFiducial()
{
    if (this->LabelText)
    {
        delete [] this->LabelText;
        this->LabelText = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  //Superclass::WriteXML(of, nIndent);

  if (this->LabelText != NULL)
  {
      of << " name=\"" << this->LabelText << "\"";
  }
  
  of << " xyz=\"" << this->XYZ[0] << " " << 
                    this->XYZ[1] << " " <<
                    this->XYZ[2] << "\"";

  of << " orientationWxyz=\"" << this->OrientationWXYZ[0] << " " << 
                                this->OrientationWXYZ[1] << " " <<
                                this->OrientationWXYZ[2] << " " << 
                                this->OrientationWXYZ[3] << "\"";
  
  of << " selected=\"" << this->Selected << "\"";
  
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::ReadXMLAttributes(const char** atts)
{

    //vtkMRMLNode::ReadXMLAttributes(atts);

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
    else if (!strcmp(attName, "labeltext"))
    {
        this->SetLabelText(attValue);
    }
    else if (!strcmp(attName, "selected"))
    {
        this->SetSelected(attValue);
    }
  }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, LabelText, ID
void vtkMRMLFiducial::Copy(vtkObject *anode)
{
//  vtkObject::Copy(anode);
  vtkMRMLFiducial *node = (vtkMRMLFiducial *) anode;

  // Vectors
  this->SetOrientationWXYZ(node->OrientationWXYZ);
  this->SetXYZ(node->XYZ);

  this->SetLabelText(node->GetLabelText());
  this->SetSelected(node->GetSelected());
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::PrintSelf(ostream& os, vtkIndent indent)
{  
  vtkObject::PrintSelf(os,indent);

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

  // LabelText:
  os << indent << "LabelText: " << (this->LabelText ? this->LabelText : "(none)") << "\n";

  os << indent << "Selected: " << this->Selected << "\n";
}


//----------------------------------------------------------------------------
void vtkMRMLFiducial::SetOrientationWXYZFromMatrix4x4(vtkMatrix4x4 *mat)
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
