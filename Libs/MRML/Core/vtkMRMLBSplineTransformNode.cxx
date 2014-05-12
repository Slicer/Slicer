/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLBSplineTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkGeneralTransform.h"
#include "vtkBSplineTransform.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkNew.h"

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLBSplineTransformNode);

//----------------------------------------------------------------------------
vtkMRMLBSplineTransformNode::vtkMRMLBSplineTransformNode()
{
  this->ReadWriteAsTransformToParent = 0;
  vtkNew<vtkBSplineTransform> warp;
  this->SetAndObserveTransformFromParent(warp.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLBSplineTransformNode::~vtkMRMLBSplineTransformNode()
{
  this->SetAndObserveTransformFromParent(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLBSplineTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLBSplineTransformNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLBSplineTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLBSplineTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

// End
