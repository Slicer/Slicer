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

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLClipModelsNode);

//----------------------------------------------------------------------------
vtkMRMLClipModelsNode::vtkMRMLClipModelsNode() = default;

//----------------------------------------------------------------------------
vtkMRMLClipModelsNode::~vtkMRMLClipModelsNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(redSliceClipState, RedSliceClipState);
  vtkMRMLReadXMLIntMacro(yellowSliceClipState, YellowSliceClipState);
  vtkMRMLReadXMLIntMacro(greenSliceClipState, GreenSliceClipState);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyIntMacro(RedSliceClipState);
  vtkMRMLCopyIntMacro(YellowSliceClipState);
  vtkMRMLCopyIntMacro(GreenSliceClipState);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLClipModelsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintIntMacro(RedSliceClipState);
  vtkMRMLPrintIntMacro(YellowSliceClipState);
  vtkMRMLPrintIntMacro(GreenSliceClipState);
  vtkMRMLPrintEndMacro();
}
