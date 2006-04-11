/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLSliceNode.h"

#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

//------------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceNode");
  if(ret)
    {
      return (vtkMRMLSliceNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLSliceNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLSliceNode");
  if(ret)
    {
      return (vtkMRMLSliceNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLSliceNode;
}

//----------------------------------------------------------------------------
// Constructor
vtkMRMLSliceNode::vtkMRMLSliceNode()
{
    // set by user
  this->SliceToRAS = vtkMatrix4x4::New();
  this->SliceToRAS->Identity();

    // calculated by UpdateMatrices()
  this->XYToSlice = vtkMatrix4x4::New();
  this->XYToRAS = vtkMatrix4x4::New();

  // set the default field of view to a convenient size for looking 
  // at slices through human heads (a 1mm thick slab 25x25 cm)
  // TODO: how to represent this as a slab rather than infinitessimal slice?
  this->SetFieldOfView(250.0, 250.0, 1.0);
  this->SetDimensions(256, 256, 1);
  this->SetOrientationToAxial();

  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode::~vtkMRMLSliceNode()
{
  if ( this->SliceToRAS != NULL) 
    {
    this->SliceToRAS->Delete();
    }
  if ( this->XYToSlice != NULL) 
    {
    this->XYToSlice->Delete();
    }
  if ( this->XYToRAS != NULL) 
    {
    this->XYToRAS->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToAxial()
{
    this->SliceToRAS->Identity();

    // Px -> Patient Left
    this->SliceToRAS->SetElement(0, 0, -1.0);
    // Py -> Patient Anterior
    this->SliceToRAS->SetElement(1, 1,  1.0);
    // Pz -> Patient Inferior
    this->SliceToRAS->SetElement(2, 2,  1.0);

    this->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToSagittal()
{
    this->SliceToRAS->Identity();

    // Px -> Patient Posterior
    this->SliceToRAS->SetElement(0, 0,  0.0);
    this->SliceToRAS->SetElement(1, 0, -1.0);
    this->SliceToRAS->SetElement(2, 0,  0.0);
    // Py -> Patient Inferior
    this->SliceToRAS->SetElement(0, 1,  0.0);
    this->SliceToRAS->SetElement(1, 1,  0.0);
    this->SliceToRAS->SetElement(2, 1,  1.0);
    // Pz -> Patient Right
    this->SliceToRAS->SetElement(0, 2,  1.0);
    this->SliceToRAS->SetElement(1, 2,  0.0);
    this->SliceToRAS->SetElement(2, 2,  0.0);

    this->UpdateMatrices();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToCoronal()
{
    this->SliceToRAS->Identity();

    // Px -> Patient Left
    this->SliceToRAS->SetElement(0, 0, -1.0);
    this->SliceToRAS->SetElement(1, 0,  0.0);
    this->SliceToRAS->SetElement(2, 0,  0.0);
    // Py -> Patient Inferior
    this->SliceToRAS->SetElement(0, 1,  0.0);
    this->SliceToRAS->SetElement(1, 1,  0.0);
    this->SliceToRAS->SetElement(2, 1,  1.0);
    // Pz -> Patient Anterior
    this->SliceToRAS->SetElement(0, 2,  0.0);
    this->SliceToRAS->SetElement(1, 2,  1.0);
    this->SliceToRAS->SetElement(2, 2,  0.0);

    this->UpdateMatrices();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::UpdateMatrices()
{
    double spacing[3];
    unsigned int i;
    this->XYToSlice->Identity();

    // the mapping from XY output slice pixels to Slice Plane coordinate
    for (i = 0; i < 3; i++)
      {
      spacing[i] = this->FieldOfView[i] / this->Dimensions[i];
      this->XYToSlice->SetElement(i, i, spacing[i]);
      this->XYToSlice->SetElement(i, 3, -this->FieldOfView[i] / 2.);
      }
      this->XYToSlice->SetElement(2, 2, 1.);
      this->XYToSlice->SetElement(2, 3, 0.);

    // the mapping from slice plane coordinates to RAS 
    // (the Orienation as in Axial, Sagittal, Coronal)
    // 
    // The combined transform:
    //
    // | R | = [Slice to RAS ] [ XY to Slice ]  | X |
    // | A |                                    | Y |
    // | S |                                    | Z |
    // | 1 |                                    | 1 |
    //
    // or
    //
    // RAS = XYToRAS * XY
    //
    
    vtkMatrix4x4::Multiply4x4(this->SliceToRAS, this->XYToSlice, this->XYToRAS);

    this->Modified();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::WriteXML(ostream& of, int nIndent)
{
  int i;

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << "fieldOfView=\"" << 
        this->FieldOfView[0] << " " <<
        this->FieldOfView[1] << " " <<
        this->FieldOfView[2] << "\" ";

  of << indent << "dimensions=\"" << 
        this->Dimensions[0] << " " <<
        this->Dimensions[1] << " " <<
        this->Dimensions[2] << "\" ";

  std::stringstream ss;
  int j;
  for (i=0; i<4; i++) 
    {
    for (j=0; j<4; j++) 
      {
      ss << this->SliceToRAS->GetElement(i,j);
      if ( !( i==3 && j==3) )
        {
        ss << " ";
        }
      }
    }
  of << indent << "sliceToRAS=\"" << ss.str() << "\" ";
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fieldOfView")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      int i;
      for (i=0; i<3; i++) 
        {
        ss >> val;
        this->FieldOfView[i] = val;
        }
      }
    if (!strcmp(attName, "dimensions")) 
      {
      std::stringstream ss;
      unsigned int val;
      ss << attValue;
      int i;
      for (i=0; i<3; i++) 
        {
        ss >> val;
        this->Dimensions[i] = val;
        }
      }
    if (!strcmp(attName, "sliceToRAS")) 
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      int i, j;
      for (i=0; i<4; i++) 
        {
        for (j=0; j<4; j++) 
          {
          ss >> val;
          this->SliceToRAS->SetElement(i,j,val);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSliceNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSliceNode *node = vtkMRMLSliceNode::SafeDownCast(anode);

  this->SliceToRAS->DeepCopy(node->GetSliceToRAS());

  int i;
  for(i=0; i<3; i++) 
    {
    this->FieldOfView[i] = node->FieldOfView[i];
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);
  os << "FieldOfView:\n ";
  for (idx = 0; idx < 3; ++idx) {
    os << indent << " " << this->FieldOfView[idx];
  }
  os << "\n";

  os << indent << "SliceToRAS: \n";
  this->SliceToRAS->PrintSelf(os, indent.GetNextIndent());
}


// End
