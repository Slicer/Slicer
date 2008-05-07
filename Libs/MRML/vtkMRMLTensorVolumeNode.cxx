/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLTensorVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMatrix4x4.h"


#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkAssignAttribute.h"
#include "vtkMRMLScalarVolumeNode.h"

//------------------------------------------------------------------------------
vtkMRMLTensorVolumeNode* vtkMRMLTensorVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTensorVolumeNode");
  if(ret)
    {
    return (vtkMRMLTensorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTensorVolumeNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLTensorVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTensorVolumeNode");
  if(ret)
    {
    return (vtkMRMLTensorVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTensorVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLTensorVolumeNode::vtkMRMLTensorVolumeNode()
{
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      this->MeasurementFrameMatrix[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
  this->Order = -1; //Tensor order

  this->DTIMathematics = NULL;
  this->AssignAttributeTensorsFromScalars = NULL;
  
}

//----------------------------------------------------------------------------
vtkMRMLTensorVolumeNode::~vtkMRMLTensorVolumeNode()
{
  if (DTIMathematics)
    {
    DTIMathematics->Delete();
    DTIMathematics = NULL;
    }
  if (AssignAttributeTensorsFromScalars)
    {
    AssignAttributeTensorsFromScalars->Delete();
    AssignAttributeTensorsFromScalars = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  std::stringstream ss;
  for(int i=0; i<3; i++)
    {
    for(int j=0; j<3; j++)
      {
      ss << this->MeasurementFrameMatrix[i][j] << " ";
      if ( i != 2 && j != 2 )
        {
        ss << "  ";
        }
      }
    }
    of << indent << " measurementFrame=\"" << ss.str() << "\"";

   of << indent << " order=\"" << Order << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "measurementFrame"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      for(int i=0; i<3; i++)
        {
        for(int j=0; j<3; j++)
          {
          ss >> val;
          this->MeasurementFrameMatrix[i][j] = val;
          }
        }
      }

    if (!strcmp(attName, "order"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Order;
      }
  }

} 

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::SetMeasurementFrameMatrix(const double mf[3][3])
{
  for (int i=0; i<3; i++) 
    {
    for (int j=0; j<3; j++) 
      {
      this->MeasurementFrameMatrix[i][j] = mf[i][j];
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::GetMeasurementFrameMatrix(double mf[3][3])
{
  for (int i=0; i<3; i++) 
    {
    for (int j=0; j<3; j++) 
      {
      mf[i][j]= this->MeasurementFrameMatrix[i][j];
      }
    }
}

void vtkMRMLTensorVolumeNode::SetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      this->MeasurementFrameMatrix[i][j]=mf->GetElement(i,j);
      }
    }
}

void vtkMRMLTensorVolumeNode::GetMeasurementFrameMatrix(vtkMatrix4x4 *mf)
{

  mf->Identity();
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      mf->SetElement(i,j,this->MeasurementFrameMatrix[i][j]);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs)
{
  MeasurementFrameMatrix[0][0] = xr;
  MeasurementFrameMatrix[0][1] = xa;
  MeasurementFrameMatrix[0][2] = xs;
  MeasurementFrameMatrix[1][0] = yr;
  MeasurementFrameMatrix[1][1] = ya;
  MeasurementFrameMatrix[1][2] = ys;
  MeasurementFrameMatrix[2][0] = zr;
  MeasurementFrameMatrix[2][1] = za;
  MeasurementFrameMatrix[2][2] = zs;
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTensorVolumeNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTensorVolumeNode *node = (vtkMRMLTensorVolumeNode *) anode;

  // Matrices
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      this->MeasurementFrameMatrix[i][j] = node->MeasurementFrameMatrix[i][j];
      }
    }
  this->Order = node->Order;
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << "MeasurementFrameMatrix:\n";
  for(int i=0; i<3; i++) 
    {
    for(int j=0; j<3; j++) 
      {
      os << indent << " " << this->MeasurementFrameMatrix[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";
  os << "Order: "<<this->Order;

}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::UpdateFromMRML()
{
  this->CalculateAutoLevels();
}

//----------------------------------------------------------------------------
void vtkMRMLTensorVolumeNode::CalculateAutoLevels(vtkMRMLVolumeDisplayNode *refNode, vtkImageData *refData)
{
  if (!refNode && !this->GetDisplayNode())
    {
    vtkDebugMacro("CalculateAutoLevels: input display node is null, and cannot get local display node");
    return;
    }

  vtkMRMLDiffusionTensorVolumeDisplayNode *displayNode;
  if (refNode == NULL)
    {
    displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
    if (!displayNode)
      {
      vtkDebugMacro("CalculateAutoLevels: this node doesn't have a volume display node, can't calculate win/level/thresh");
      return;
      }
    }
  else
    {
    displayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(refNode);
    }

  if (displayNode == NULL)
    {
    vtkWarningMacro("CalculateAutoLevels: unable to get a dt volume display node.");
    return;
    }

  if (!displayNode->GetAutoWindowLevel())
    {
    vtkDebugMacro("CalculateAutoLevels: " << (this->GetID() == NULL ? "nullid" : this->GetID()) << ": Auto window level not turned on, returning.");
    return;
    }
    
  vtkImageData *imageDataScalar;
  if (refData == NULL)
    {
    // this is going to fail, for tensors, we really need to have the image
    // data passed in
    imageDataScalar = this->GetImageData();
    }
  else
    {
    imageDataScalar = refData;
    }

  if ( !imageDataScalar )
    {
    vtkDebugMacro("CalculateAutoLevels: image data is null");
    return;
    }

  if (displayNode != NULL ) 
    {
    if (displayNode->GetDiffusionTensorDisplayPropertiesNode())
      {
      if (this->AssignAttributeTensorsFromScalars == NULL)
        {
        this->AssignAttributeTensorsFromScalars = vtkAssignAttribute::New();
        }
      if (this->DTIMathematics == NULL)
        {
        this->DTIMathematics = vtkDiffusionTensorMathematics::New();
        }
      this->AssignAttributeTensorsFromScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
      
      this->DTIMathematics->SetInput(imageDataScalar);
      this->DTIMathematics->SetOperation(displayNode->GetDiffusionTensorDisplayPropertiesNode()->
                                   GetScalarInvariant());
      this->DTIMathematics->Update();
      imageDataScalar = this->DTIMathematics->GetOutput();
      }
    else
      {
      imageDataScalar = NULL;
      }
    }
  if (imageDataScalar != NULL)
    {
    // pass it up to the superclass
    this->CalculateScalarAutoLevels(displayNode, imageDataScalar);
    }
}

