/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGridTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLGridTransformNode.h"

// VTK includes
#include <vtkOrientedGridTransform.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLGridTransformNode);

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::vtkMRMLGridTransformNode()
{
  // Set up the node with a dummy displacement field (that contains one single
  // null-vector) to make sure the node is valid and can be saved
  vtkNew<vtkImageData> emptyDisplacementField;
  emptyDisplacementField->SetDimensions(1,1,1);
  emptyDisplacementField->AllocateScalars(VTK_DOUBLE, 3);
  emptyDisplacementField->SetScalarComponentFromDouble(0,0,0, 0, 0.0);
  emptyDisplacementField->SetScalarComponentFromDouble(0,0,0, 1, 0.0);
  emptyDisplacementField->SetScalarComponentFromDouble(0,0,0, 2, 0.0);

  vtkNew<vtkOrientedGridTransform> warp;
  warp->SetDisplacementGridData( emptyDisplacementField.GetPointer() );

  this->SetAndObserveTransformFromParent(warp.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::~vtkMRMLGridTransformNode()
{
  this->SetAndObserveTransformFromParent(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}
