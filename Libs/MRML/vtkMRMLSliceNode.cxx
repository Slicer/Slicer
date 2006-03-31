/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

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
  this->RASToSlice = vtkMatrix4x4::New();
  this->RASToSlice->Identity();

    // calculated by UpdateMatrices()
  this->XYToSlice = vtkMatrix4x4::New();
  this->XYToRAS = vtkMatrix4x4::New();

  // set the default field of view to a convenient size for looking 
  // at slices through human heads (a 1mm thick slab 25x25 cm)
  this->SetFieldOfView(250.0, 250.0, 1.0);
  this->SetDimensions(256, 256, 1);
  this->SetOrientationToAxial();

  this->UpdateMatrices();
}

//----------------------------------------------------------------------------
vtkMRMLSliceNode::~vtkMRMLSliceNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToAxial()
{
    this->RASToSlice->Identity();
    // Px -> Patient Left
    this->RASToSlice->SetElement(0, 0, -1.0);
    // Py -> Patient Anterior
    this->RASToSlice->SetElement(1, 1,  1.0);
    // Pz -> Patient Inferior
    this->RASToSlice->SetElement(1, 1, -1.0);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToSagittal()
{
    vtkErrorMacro("Not yet implemented");
    this->RASToSlice->Identity();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::SetOrientationToCoronal()
{
    vtkErrorMacro("Not yet implemented");
    this->RASToSlice->Identity();
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

    // the mapping from slice plane coordinates to RAS 
    // (the Orienation as in Axial, Sagittal, Coronal)
    vtkMatrix4x4 *SliceToRAS = vtkMatrix4x4::New();
    SliceToRAS->DeepCopy(this->RASToSlice);
    SliceToRAS->Invert();

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

    vtkMatrix4x4::Multiply4x4(SliceToRAS, this->XYToSlice, this->XYToRAS);
    
    SliceToRAS->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLSliceNode::WriteXML(ostream& of, int nIndent)
{
  int i;

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << "FieldOfView='" << 
        this->FieldOfView[0] << " " <<
        this->FieldOfView[1] << " " <<
        this->FieldOfView[2] << "' ";

  of << indent << "Dimensions='" << 
        this->Dimensions[0] << " " <<
        this->Dimensions[1] << " " <<
        this->Dimensions[2] << "' ";

  std::stringstream ss;
  int j;
  for (i=0; i<4; i++) 
    {
    for (j=0; j<4; j++) 
      {
      ss << this->RASToSlice->GetElement(i,j);
      if ( !( i==3 && j==3) )
        {
        ss << " ";
        }
      }
    }
  of << indent << "RASToSlice='" << ss.str() << "' ";
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
    if (!strcmp(attName, "FieldOfView")) 
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
    if (!strcmp(attName, "Dimensions")) 
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
    if (!strcmp(attName, "RASToSlice")) 
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
          this->RASToSlice->SetElement(i,j,val);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLSliceNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLSliceNode *node = vtkMRMLSliceNode::SafeDownCast(anode);

  this->RASToSlice->DeepCopy(node->GetRASToSlice());

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

  os << indent << "RASToSlice: \n";
  this->RASToSlice->PrintSelf(os, indent.GetNextIndent());
}


// End
