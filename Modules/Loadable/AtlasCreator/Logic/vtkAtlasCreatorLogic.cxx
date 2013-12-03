/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkAtlasCreatorLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Slicer includes
#include "vtkAtlasCreatorLogic.h"

// VTKITK includes

// MRML includes

// VTK includes
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAtlasCreatorLogic);


//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::vtkAtlasCreatorLogic()
{
  this->AtlasCreatorNode = NULL;
}

//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::~vtkAtlasCreatorLogic()
{
  vtkSetMRMLNodeMacro(this->AtlasCreatorNode, NULL);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorLogic::SetAndObserveAtlasCreatorNode(vtkMRMLAtlasCreatorNode *n)
{
  vtkSetAndObserveMRMLNodeMacro( this->AtlasCreatorNode, n);
}

void vtkAtlasCreatorLogic::Apply()
{
}

