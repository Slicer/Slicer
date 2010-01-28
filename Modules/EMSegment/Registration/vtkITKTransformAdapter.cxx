/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkITKTransformAdapter.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkITKTransformAdapter.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkITKTransformAdapter, "$Revision: 1.30 $");
vtkStandardNewMacro(vtkITKTransformAdapter);

void vtkITKTransformAdapter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << "ITKTransform: (not implemented)" << std::endl;
}

