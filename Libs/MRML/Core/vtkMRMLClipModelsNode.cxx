/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLClipModelsNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLClipModelsNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLClipModelsNode);

//----------------------------------------------------------------------------
vtkMRMLClipModelsNode::vtkMRMLClipModelsNode()
{
  this->SetSingletonTag("vtkMRMLClipModelsNode");
  this->HideFromEditors = true;
  this->ClipType = 0;
  this->RedSliceClipState = 0;
  this->YellowSliceClipState = 0;
  this->GreenSliceClipState = 0;
}

//----------------------------------------------------------------------------
vtkMRMLClipModelsNode::~vtkMRMLClipModelsNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " clipType=\"" << this->ClipType << "\"";

  of << indent << " redSliceClipState=\"" << this->RedSliceClipState << "\"";
  of << indent << " yellowSliceClipState=\"" << this->YellowSliceClipState << "\"";
  of << indent << " greenSliceClipState=\"" << this->GreenSliceClipState << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "yellowSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> YellowSliceClipState;
      }
    else if (!strcmp(attName, "redSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> RedSliceClipState;
      }
    else if (!strcmp(attName, "greenSliceClipState"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> GreenSliceClipState;
      }
    else if (!strcmp(attName, "clipType"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ClipType;
      }
    }
    this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLClipModelsNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLClipModelsNode *node = (vtkMRMLClipModelsNode *) anode;

  this->SetClipType(node->ClipType);
  this->SetYellowSliceClipState(node->YellowSliceClipState);
  this->SetGreenSliceClipState(node->GreenSliceClipState);
  this->SetRedSliceClipState(node->RedSliceClipState);

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ClipType:        " << this->ClipType << "\n";
  os << indent << "YellowSliceClipState: " << this->YellowSliceClipState << "\n";
  os << indent << "GreenSliceClipState:  " << this->GreenSliceClipState << "\n";
  os << indent << "RedSliceClipState:    " << this->RedSliceClipState << "\n";
}

