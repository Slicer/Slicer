/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLBSplineTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkMRMLBSplineTransformNode.h"

#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkOrientedBSplineTransform.h"
#include "vtkNew.h"

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLBSplineTransformNode);

//----------------------------------------------------------------------------
vtkMRMLBSplineTransformNode::vtkMRMLBSplineTransformNode()
{
  // Set up the node with a dummy bspline grid (that contains a small set of
  // null-vectors) to make sure the node is valid and can be saved
  double gridSize[3]={4,4,4};
  vtkNew<vtkImageData> bsplineCoefficients;
  bsplineCoefficients->SetExtent(0, gridSize[0]-1, 0, gridSize[1]-1, 0, gridSize[2]-1);
  bsplineCoefficients->AllocateScalars(VTK_DOUBLE, 3);
  double* bsplineParams=static_cast<double*>(bsplineCoefficients->GetScalarPointer());
  const unsigned int numberOfParams = 3*gridSize[0]*gridSize[1]*gridSize[2];
  for (unsigned int i=0; i<numberOfParams; i++)
    {
    *(bsplineParams++) =  0.0;
    }

  vtkNew<vtkOrientedBSplineTransform> warp;
  warp->SetCoefficientData(bsplineCoefficients.GetPointer());

  this->SetAndObserveTransformFromParent(warp.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLBSplineTransformNode::~vtkMRMLBSplineTransformNode()
{
  this->SetAndObserveTransformFromParent(nullptr);
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
void vtkMRMLBSplineTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}
