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
  this->ClippingMethod = vtkMRMLClipModelsNode::Straight;
}

//----------------------------------------------------------------------------
vtkMRMLClipModelsNode::~vtkMRMLClipModelsNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " clipType=\"" << this->ClipType << "\"";

  of << " redSliceClipState=\"" << this->RedSliceClipState << "\"";
  of << " yellowSliceClipState=\"" << this->YellowSliceClipState << "\"";
  of << " greenSliceClipState=\"" << this->GreenSliceClipState << "\"";
  if (this->ClippingMethod != vtkMRMLClipModelsNode::Straight)
    {
    of << " clippingMethod=\"" << (this->GetClippingMethodAsString(this->ClippingMethod)) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
    else if (!strcmp(attName, "clippingMethod"))
      {
      std::stringstream ss;
      ss << attValue;
      int id = this->GetClippingMethodFromString(attValue);
      if (id < 0)
        {
        vtkWarningMacro("Invalid Clipping Methods: "<<(attValue?attValue:"(none)"));
        }
      else
        {
        this->ClippingMethod = static_cast<ClippingMethodType>(id);
        }
      }
    }
    this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLClipModelsNode* node = vtkMRMLClipModelsNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetClipType(node->ClipType);
  this->SetYellowSliceClipState(node->YellowSliceClipState);
  this->SetGreenSliceClipState(node->GreenSliceClipState);
  this->SetRedSliceClipState(node->RedSliceClipState);
  this->SetClippingMethod(node->ClippingMethod);
}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ClipType:        " << this->ClipType << "\n";
  os << indent << "YellowSliceClipState: " << this->YellowSliceClipState << "\n";
  os << indent << "GreenSliceClipState:  " << this->GreenSliceClipState << "\n";
  os << indent << "RedSliceClipState:    " << this->RedSliceClipState << "\n";
  os << indent << " clippingMethod=\"" << (this->GetClippingMethodAsString(this->ClippingMethod)) << "\n";
}

//-----------------------------------------------------------------------------
int vtkMRMLClipModelsNode::GetClippingMethodFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  if (strcmp(name, "Straight"))
    {
    return (int)Straight;
    }
  else if (strcmp(name, "Whole Cells"))
    {
    return (int)WholeCells;
    }
  else if (strcmp(name, "Whole Cells With Boundary"))
    {
    return (int)WholeCellsWithBoundary;
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkMRMLClipModelsNode::GetClippingMethodAsString(ClippingMethodType id)
{
 switch (id)
    {
    case Straight: return "Straight";
    case WholeCells: return "Whole Cells";
    case WholeCellsWithBoundary: return "Whole Cells With Boundary";
    default:
      // invalid id
      return "";
    }
}

