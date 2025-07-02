/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCrosshairNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCrosshairNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCrosshairNode);

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::vtkMRMLCrosshairNode()
{
  this->HideFromEditors = 1;
  this->SetSingletonTag("default");
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::~vtkMRMLCrosshairNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(crosshairMode, CrosshairMode);
  vtkMRMLWriteXMLEnumMacro(crosshairBehavior, CrosshairBehavior);
  vtkMRMLWriteXMLEnumMacro(crosshairThickness, CrosshairThickness);
  vtkMRMLWriteXMLVectorMacro(crosshairRAS, CrosshairRAS, double, 3);
  // This property is only for evaluation of this feature and the value is not stored persistently in the scene file.
  // vtkMRMLWriteXMLBooleanMacro(fastPick3D, FastPick3D);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(crosshairMode, CrosshairMode);
  vtkMRMLReadXMLEnumMacro(crosshairBehavior, CrosshairBehavior);
  vtkMRMLReadXMLEnumMacro(crosshairThickness, CrosshairThickness);
  vtkMRMLReadXMLVectorMacro(crosshairRAS, CrosshairRAS, double, 3);
  // This property is only for evaluation of this feature and the value is not stored persistently in the scene file.
  // vtkMRMLReadXMLBooleanMacro(fastPick3D, FastPick3D);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCrosshairNode* node = vtkMRMLCrosshairNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyEnumMacro(CrosshairMode);
  vtkMRMLCopyEnumMacro(CrosshairBehavior);
  vtkMRMLCopyEnumMacro(CrosshairThickness);
  vtkMRMLCopyVectorMacro(CrosshairRAS, double, 3);
  vtkMRMLCopyBooleanMacro(FastPick3D);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CrosshairMode);
  vtkMRMLPrintEnumMacro(CrosshairBehavior);
  vtkMRMLPrintEnumMacro(CrosshairThickness);
  vtkMRMLPrintVectorMacro(CrosshairRAS, double, 3);
  vtkMRMLPrintBooleanMacro(FastPick3D);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCrosshairRAS(double ras[3], int id)
{
  bool modified = false;

  if (this->LightBoxPane != id)
  {
    modified = true;
  }

  if (this->CrosshairRAS[0] != ras[0] || this->CrosshairRAS[1] != ras[1] || this->CrosshairRAS[2] != ras[2])
  {
    modified = true;
  }

  this->CrosshairRAS[0] = ras[0];
  this->CrosshairRAS[1] = ras[1];
  this->CrosshairRAS[2] = ras[2];
  this->LightBoxPane = id;

  if (modified)
  {
    this->Modified();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionRAS(double ras[3])
{
  this->CursorPositionRAS[0] = ras[0];
  this->CursorPositionRAS[1] = ras[1];
  this->CursorPositionRAS[2] = ras[2];
  this->CursorPositionRASValid = true;
  this->CursorSliceNode = nullptr; // slice position is not available
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionXYZ(double xyz[3], vtkMRMLSliceNode* sliceNode)
{
  this->CursorPositionXYZ[0] = xyz[0];
  this->CursorPositionXYZ[1] = xyz[1];
  this->CursorPositionXYZ[2] = xyz[2];
  this->CursorSliceNode = sliceNode;

  // Cursor position in the slice viewer defines the RAS position, so update that as well
  if (this->CursorSliceNode)
  {
    double xyzw[4] = { xyz[0], xyz[1], xyz[2], 1.0 };
    double rasw[4] = { 0.0, 0.0, 0.0, 1.0 };
    sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rasw);
    this->CursorPositionRAS[0] = rasw[0] / rasw[3];
    this->CursorPositionRAS[1] = rasw[1] / rasw[3];
    this->CursorPositionRAS[2] = rasw[2] / rasw[3];
    this->CursorPositionRASValid = true;
  }

  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionInvalid()
{
  this->CursorPositionRASValid = false;
  this->CursorSliceNode = nullptr;
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairNode::GetCursorPositionRAS(double ras[3])
{
  ras[0] = this->CursorPositionRAS[0];
  ras[1] = this->CursorPositionRAS[1];
  ras[2] = this->CursorPositionRAS[2];
  return this->CursorPositionRASValid;
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLCrosshairNode::GetCursorPositionXYZ(double xyz[3])
{
  xyz[0] = this->CursorPositionXYZ[0];
  xyz[1] = this->CursorPositionXYZ[1];
  xyz[2] = this->CursorPositionXYZ[2];
  return this->CursorSliceNode;
}

//-----------------------------------------------------------
const char* vtkMRMLCrosshairNode::GetCrosshairModeAsString(int id)
{
  switch (id)
  {
    case vtkMRMLCrosshairNode::NoCrosshair:
      return "NoCrosshair";
    case vtkMRMLCrosshairNode::ShowBasic:
      return "ShowBasic";
    case vtkMRMLCrosshairNode::ShowIntersection:
      return "ShowIntersection";
    case vtkMRMLCrosshairNode::ShowHashmarks:
      return "ShowHashmarks";
    case vtkMRMLCrosshairNode::ShowAll:
      return "ShowAll";
    case vtkMRMLCrosshairNode::ShowSmallBasic:
      return "ShowSmallBasic";
    case vtkMRMLCrosshairNode::ShowSmallIntersection:
      return "ShowSmallIntersection";
    default:
      // invalid id
      return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLCrosshairNode::GetCrosshairModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < CrosshairMode_Last; i++)
  {
    if (strcmp(name, vtkMRMLCrosshairNode::GetCrosshairModeAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLCrosshairNode::GetCrosshairBehaviorAsString(int id)
{
  switch (id)
  {
    case vtkMRMLCrosshairNode::OffsetJumpSlice:
      return "OffsetJumpSlice";
    case vtkMRMLCrosshairNode::CenteredJumpSlice:
      return "CenteredJumpSlice";
    case vtkMRMLCrosshairNode::NoAction:
      return "NoAction";
    default:
      // invalid id
      return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLCrosshairNode::GetCrosshairBehaviorFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < CrosshairBehavior_Last; i++)
  {
    if (strcmp(name, vtkMRMLCrosshairNode::GetCrosshairBehaviorAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // Alternative names for OffsetJumpSlice for legacy scenes
  if (!strcmp(name, "JumpSlice") || !strcmp(name, "Normal"))
  {
    return vtkMRMLCrosshairNode::OffsetJumpSlice;
  }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLCrosshairNode::GetCrosshairThicknessAsString(int id)
{
  switch (id)
  {
    case vtkMRMLCrosshairNode::Fine:
      return "Fine";
    case vtkMRMLCrosshairNode::Medium:
      return "Medium";
    case vtkMRMLCrosshairNode::Thick:
      return "Thick";
    default:
      // invalid id
      return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLCrosshairNode::GetCrosshairThicknessFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < CrosshairThickness_Last; i++)
  {
    if (strcmp(name, vtkMRMLCrosshairNode::GetCrosshairThicknessAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
