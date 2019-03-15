/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducial.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include <vtkMatrix4x4.h>

#include "vtkMRMLFiducial.h"
//#include "vtkMRMLFiducialListNode.h"
//#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLFiducial);

//----------------------------------------------------------------------------
vtkMRMLFiducial::vtkMRMLFiducial()
{
  this->XYZ[0] = this->XYZ[1] = this->XYZ[2] = 0.0;
  this->OrientationWXYZ[0] = this->OrientationWXYZ[1] = this->OrientationWXYZ[2]  = 0.0;
  this->OrientationWXYZ[3] = 1.0;
  // so that the SetLabelText macro won't try to free memory
  this->LabelText = nullptr;
  this->SetLabelText("");
  this->ID = nullptr;
  this->SetID("");
  this->Selected = false;
  this->Visibility = true;
}

//----------------------------------------------------------------------------
vtkMRMLFiducial::~vtkMRMLFiducial()
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
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::WriteXML(ostream& of, int vtkNotUsed(nIndent))
{
  // this is encapsulated in the fiducial list storage node, but needs to be
  // here as well because when scene snapshots are restored, storage nodes are
  // not re-read

  // Write all attributes, since the parsing of the string is dependent on the
  // order here

  //Superclass::WriteXML(of, nIndent);


  // now that it's not a first class node, write it out simply
  if (this->ID != nullptr)
    {
    of << "id " << this->ID;
    }
  if (this->LabelText != nullptr)
    {
    of << " labeltext " << this->LabelText;
    }

  of << " xyz " << this->XYZ[0] << " " <<
                    this->XYZ[1] << " " <<
                    this->XYZ[2];

  of << " orientationwxyz " << this->OrientationWXYZ[0] << " " <<
                                this->OrientationWXYZ[1] << " " <<
                                this->OrientationWXYZ[2] << " " <<
                                this->OrientationWXYZ[3];

  of << " selected " << this->Selected;
  of << " visibility " << this->Visibility;
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::ReadXMLString(const char *keyValuePairs)
{
    // used because the fiducial list gloms together the point's key and
    // values into one long string, VERY dependent on the order it's written
    // out in when WriteXML is used

    // insert the string into a stream
    std::stringstream ss;
    ss << keyValuePairs;

    std::string keyName;

    // get out the id
    ss >> keyName;
    if (keyName == std::string("id"))
      {
      //ss >> this->ID;
      ss >> keyName;
      this->SetID(keyName.c_str());

      vtkDebugMacro("ReadXMLString: got id " << this->ID);
      }

    // now get out the labeltext key
    ss >> keyName;
    bool emptyLabel = false;
    if (keyName == std::string("labeltext"))
      {
      // now get the label text value
      ss >> keyName;
      // check for an empty string
      if (keyName.compare("xyz") == 0)
        {
        // we have an empty label
        this->SetLabelText("");
        // set a flag
        emptyLabel = true;
        }
      else
        {
        this->SetLabelText(keyName.c_str());
        }


      vtkDebugMacro("ReadXMLString: got label text '" << this->LabelText << "'");
      }



    if (!emptyLabel)
      {
      // get the xyz key
      ss >> keyName;
      vtkDebugMacro("ReadXMLString: after checking labeltext, keyname = " << keyName.c_str());

      while (keyName != std::string("xyz") &&
             keyName != std::string(""))
        {
        // we're still getting parts of the labeltext
        std::string newLabel = std::string(this->GetLabelText()) + std::string(" ") + keyName;
        vtkDebugMacro("ReadXMLString: adding to label text: '" << newLabel.c_str() << "'");
        this->SetLabelText(newLabel.c_str());
        ss >> keyName;
        }
      }
    if (emptyLabel ||
        keyName == std::string("xyz"))
      {
      // now get the x, y, z values
      ss >> this->XYZ[0];
      ss >> this->XYZ[1];
      ss >> this->XYZ[2];
      vtkDebugMacro("ReadXMLString: got xyz: " <<  this->XYZ[0] << ", " << this->XYZ[1] << ", " <<  this->XYZ[2]);
      }

    // get the orientation key
    ss >> keyName;
    vtkDebugMacro("ReadXMLString: got keyname " << keyName.c_str());

    if (keyName == std::string("orientationwxyz"))
      {
      // now get the w, x, y, z values
      ss >> this->OrientationWXYZ[0];
      ss >> this->OrientationWXYZ[1];
      ss >> this->OrientationWXYZ[2];
      ss >> this->OrientationWXYZ[3];
      vtkDebugMacro("ReadXMLString: got wxyz: " <<  this->OrientationWXYZ[0] << ", " << this->OrientationWXYZ[1] << ", " <<  this->OrientationWXYZ[2] << ", " <<  this->OrientationWXYZ[3]);
      }

    // get the selected flag
    ss >> keyName;
    if (keyName == std::string("selected"))
      {
      ss >> this->Selected;
      vtkDebugMacro("ReadXMLString: got keynamne = " << keyName.c_str() << ", selected = " << this->Selected );
      }

    // get the visibility flag
    ss >> keyName;
    if (keyName == std::string("visibility"))
      {
      vtkDebugMacro("ReadXMLString: got keyName = " << keyName.c_str() << ", visibility " << this->Visibility);
      ss >> this->Visibility;
      }
}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::ReadXMLAttributes(const char** atts)
{
    //vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  //std::cout << "vtkMRMLFiducial::ReadXMLAttributes\n";

  while (*atts != nullptr)
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
    else if (!strcmp(attName, "id"))
    {
        this->SetID(attValue);
    }
    else if (!strcmp(attName, "labeltext"))
    {
        this->SetLabelText(attValue);
    }
    else if (!strcmp(attName, "selected"))
    {
        std::stringstream ss;
        ss << attValue;
        int sel;
        ss >> sel;
        std::cout << "selected value = " << attValue << ", sel = " << sel << endl;
        if (sel == 1)
          {
          this->SetSelected(true);
          }
        else
          {
          this->SetSelected(false);
          }
    }
    else if (!strcmp(attName, "visibility"))
    {
        std::stringstream ss;
        ss << attValue;
        int vis;
        ss >> vis;
        std::cout << "visibility value = " << attValue << ", vis = " << vis << endl;
        if (vis == 1)
          {
          this->SetVisibility(true);
          }
        else
          {
          this->SetVisibility(false);
          }
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
  this->SetVisibility(node->GetVisibility());



}

//----------------------------------------------------------------------------
void vtkMRMLFiducial::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  // ID
  os << indent << "ID: " << (this->ID ? this->ID : "(none)") << "\n";

  // LabelText:
  os << indent << "LabelText: " << (this->LabelText ? this->LabelText : "(none)") << "\n";

  // location
  os << indent << "XYZ: (";
  os << this->XYZ[0] << ", " << this->XYZ[1] << ", " << this->XYZ[2]
     << ") \n" ;

  // OrientationWXYZ
  os << indent << "OrientationWXYZ: (";
  os << this->OrientationWXYZ[0] << ", " ;
  os << this->OrientationWXYZ[1] << ", " ;
  os << this->OrientationWXYZ[2] << ", " ;
  os << this->OrientationWXYZ[3] << ")" << "\n";

  // selected flag
  os << indent << "Selected: " << this->Selected << "\n";
  // visibility flag
  os << indent << "Visibility: " << this->Visibility << "\n";
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
    {
      ortho[0][2] = -ortho[0][2];
      ortho[1][2] = -ortho[1][2];
      ortho[2][2] = -ortho[2][2];
    }

    vtkMath::Matrix3x3ToQuaternion(ortho, wxyz);

    // calc the return value wxyz
    double mag = sqrt(wxyz[1]*wxyz[1] + wxyz[2]*wxyz[2] + wxyz[3]*wxyz[3]);

    if (mag)
      {
      wxyz[0] = 2.0*acos(wxyz[0]) / vtkMath::RadiansFromDegrees(1.0);
      wxyz[1] /= mag;
      wxyz[2] /= mag;
      wxyz[3] /= mag;
      }
    else
      {
      wxyz[0] = 0.0;
      wxyz[1] = 0.0;
      wxyz[2] = 0.0;
      wxyz[3] = 1.0;
      }
    this->OrientationWXYZ[0] = (float) wxyz[0];
    this->OrientationWXYZ[1] = (float) wxyz[1];
    this->OrientationWXYZ[2] = (float) wxyz[2];
    this->OrientationWXYZ[3] = (float) wxyz[3];
}
